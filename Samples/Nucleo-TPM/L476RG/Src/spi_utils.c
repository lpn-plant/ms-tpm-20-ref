#include "spi_utils.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "TpmDevice.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define SPI_MAX_CHUNK_SIZE 80 /* how many bytes we can send/receive in a single transfer */
#define TPM_PACKET_HEADER_SIZE 4

extern SPI_HandleTypeDef hspi2;

static uint8_t rx_spi_buf[MAX_TPM_MESSAGE_SIZE];
static uint8_t response_buf[MAX_TPM_MESSAGE_SIZE];
static uint8_t cmd_buf[MAX_TPM_MESSAGE_SIZE];
static size_t cmd_buf_len;
static size_t rx_buf_len;
static size_t response_buf_len;
static size_t response_buf_offset;

static uint8_t *spi_tx_buf = NULL;
static size_t spi_tx_buf_len = 0;


HAL_StatusTypeDef spi_receive() {
  size_t offset = rx_buf_len;
  size_t left = sizeof rx_spi_buf - rx_buf_len;

  assert(rx_buf_len <= sizeof rx_spi_buf);
  assert(left <= sizeof rx_spi_buf);

  if (left == 0) {
	  fprintf(stderr, "RX buffer is full");
	  return HAL_ERROR;
  }

  HAL_StatusTypeDef hstatus = HAL_SPI_Receive(&hspi2, &rx_spi_buf[offset], left, 0);
  if (hstatus == HAL_OK || hstatus == HAL_TIMEOUT) {
	  uint16_t received = hspi2.RxXferSize - hspi2.RxXferCount;
	  assert(received <= left);
  	  rx_buf_len += received;

  	  return received > 0 ? HAL_OK : HAL_TIMEOUT;
  } else {
	  fprintf(stderr, "SPI receive failed: %d\n", hstatus);
	  return hstatus;
  }
}

HAL_StatusTypeDef spi_transmit() {
  HAL_StatusTypeDef hstatus = HAL_SPI_TransmitReceive(&hspi2, spi_tx_buf, rx_spi_buf, spi_tx_buf_len, 500);
  // Ignore HAL error - for some reason HAL_SPI_TransmitReceive returns error even if transfer succeeds
  if (hstatus != HAL_TIMEOUT)
	  spi_tx_buf = NULL;
  else
	  fprintf(stderr, "SPI TX error: %d\n", hstatus);
  return hstatus;
}

static void rx_discard() {
	rx_buf_len = 0;
}

static bool spi_tx_queue(uint8_t *buf, size_t size) {
	if (spi_tx_buf) {
		fprintf(stderr, "Previous SPI transfer not completed yet\n");
		return false;
	}

	spi_tx_buf = buf;
	spi_tx_buf_len = size;
	return true;
}

static void spi_exec(uint32_t r, uint8_t *data, uint32_t size) {
	switch (r) {
	case 0: {
		// Write TPM command into buffer
		uint32_t left = sizeof cmd_buf - cmd_buf_len;
		uint32_t n = MIN(size, left);
		memcpy(&cmd_buf[cmd_buf_len], data, n);
		cmd_buf_len += n;
		break;
	}
	case 1:
		data = cmd_buf;
		size = cmd_buf_len;
		cmd_buf_len = 0;
		// Discard any unread response
		response_buf_len = 0;
		response_buf_offset = 0;

		fprintf(stderr, "TPM command:");
		for (uint32_t i = 0; i < size; i++)
			fprintf(stderr, " %02x", data[i]);
		fprintf(stderr, "\n");

		if (!TpmSignalEvent_tmp(data, &size)) {
			fprintf(stderr, "TpmSignalEvent_tmp failed \r\n");
			return;
		} else {
			size_t response_size = MAX_TPM_MESSAGE_SIZE;
			if(!TpmOperationsLoop_tmp(data, size, response_buf, &response_size)) {
				fprintf(stderr, "TpmOperationsLoop_tmp failed\r\n");
				Error_Handler();
			}

			fprintf(stderr, "Response size: %d\n", response_size);
			response_buf_len = response_size;
		}
		break;
	case 2: {
		if (response_buf_len > 0) {
			uint32_t left = response_buf_len - response_buf_offset;
			fprintf(stderr, "Send response: left=%d\n", left);
			if (spi_tx_queue(&response_buf[response_buf_offset], left)) {
				response_buf_offset += left;
			}
		} else {
			fprintf(stderr, "No response to send");
		}
		if (response_buf_offset == response_buf_len) {
			response_buf_offset = 0;
			response_buf_len = 0;
		}
		break;
	}
	case 3: {
		fprintf(stderr, "sending test response\n");
		// For testing only
		static uint8_t test_response[] = {
			0x42, 0x43, 0xab, 0xcd, 0xde, 0xef
		};
		spi_tx_queue(test_response, sizeof test_response);
		break;
	}
	case 4: {
		spi_tx_queue((uint8_t*)&response_buf_len, sizeof response_buf_len);
		break;
	}
	default:
		fprintf(stderr, "Unknown request %ld\n", r);
		break;
	}
}

void spi_main_loop() {
	HAL_StatusTypeDef r;
	uint32_t total_cmd_size;

	rx_buf_len = 0;
	total_cmd_size = 0;
	cmd_buf_len = 0;
	response_buf_len = 0;
	response_buf_offset = 0;

	while (1) {
		if (spi_tx_buf) {
			spi_transmit();
			// Ignore any incoming data while transmitting to host.
			// Host should send send dummy bytes (0xff) till entire response is received.
			rx_discard();
			total_cmd_size = 0;
			continue;
		}

		r = spi_receive();
		if (r != HAL_OK) {
			continue;
		}

		if (rx_buf_len >= TPM_PACKET_HEADER_SIZE && total_cmd_size == 0) {
			// Ignore invalid packets to prevent hang
			if (rx_spi_buf[0] == 0xff || rx_spi_buf[1] == 0xff) {
				rx_discard();
				continue;
			}

			total_cmd_size = rx_spi_buf[0];
			if (total_cmd_size + TPM_PACKET_HEADER_SIZE > MAX_TPM_MESSAGE_SIZE) {
				rx_discard();
				continue;
			}
		}

		if (rx_buf_len >= TPM_PACKET_HEADER_SIZE + total_cmd_size) {
			uint8_t *cmd_buf = &rx_spi_buf[TPM_PACKET_HEADER_SIZE];
			spi_exec(rx_spi_buf[1], cmd_buf, total_cmd_size);
			total_cmd_size = 0;
			rx_discard();
		}
	}
}
