#include "spi_utils.h"
#include <assert.h>
#include <stdbool.h>
#include "TpmDevice.h"

extern SPI_HandleTypeDef hspi2;

uint8_t rx_spi_buf[512] = {0};
uint8_t tx_spi_buf[MAX_TPM_MESSAGE_SIZE] = {0};

HAL_StatusTypeDef spi_receive(uint16_t size) {
  assert(size <= sizeof rx_spi_buf);
  HAL_StatusTypeDef hstatus = HAL_SPI_Receive(&hspi2, rx_spi_buf, size, 500);
  return hstatus;
}

HAL_StatusTypeDef spi_transmit(uint8_t *input, size_t size) {
  uint8_t rx_data = 0;
  HAL_StatusTypeDef hstatus = HAL_SPI_TransmitReceive(&hspi2, input, &rx_data, size, 500);
  return hstatus;
}
