/*
 * cdc_get_data.c
 *
 *  Created on: Oct 27, 2021
 *      Author: acwiek
 */

#include <cdc_data.h>
#include "circular_buffer.h"
#include "stm32l4xx_hal.h"
#include "TpmDevice.h"
#include <stdio.h>

#define CMD_MAX_BUFFER_SIZE 256

static uint8_t uart_buffer[CMD_MAX_BUFFER_SIZE] = { };
static uint8_t cbuf_cmd_buffer[MAX_TPM_MESSAGE_SIZE] = { };
static cbuf_handle_t cbuf_handle;

void cdc_put_data(uint8_t *buffer, uint32_t size) {
  for (int i = 0; i < size; i++) {
    circular_buf_put(cbuf_handle, buffer[i]);
  }
}

void cdc_get_data(cdc_input_data *input) {
  input->avail = circular_buf_size(cbuf_handle);
  input->data = cbuf_cmd_buffer;

  for (int i = 0; i < input->avail; i++) {
    __disable_irq();
    int ret = circular_buf_get(cbuf_handle, &input->data[i]);
    __enable_irq();
    if (ret != 0) {
      fprintf(stderr, "ret != 0) %d \r\n", ret);
      Error_Handler();
    }
  }
}

void cdc_buffer_init() {
  cbuf_handle = circular_buf_init(uart_buffer, CMD_MAX_BUFFER_SIZE);
}
