#include "spi_utils.h"
#include <assert.h>

extern SPI_HandleTypeDef hspi2;

uint8_t rx_spi_buf[512] = {0};
uint8_t tx_spi_buf[512] = {0};

HAL_StatusTypeDef spi_receive(uint16_t size) {
  assert(size <= sizeof rx_spi_buf);
  HAL_StatusTypeDef hstatus = HAL_SPI_Receive(&hspi2, rx_spi_buf, size, 500);
  return hstatus;
}

int spi_transmit(uint8_t *input, size_t size) {
  int ret = -1;
  uint8_t rx_data = 0;
  HAL_StatusTypeDef hstatus = HAL_SPI_TransmitReceive(&hspi2, input, &rx_data, size, 500);
  if (hstatus == HAL_OK) {
    ret = rx_spi_buf[0];
  }
  else {
  ;
//    printf("failed\n");
  }

  return ret;
}
