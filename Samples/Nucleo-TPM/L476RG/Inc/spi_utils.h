#ifndef __SPI_UTILS_H__
#define __SPI_UTILS_H__

#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

extern uint8_t rx_spi_buf[];
extern uint8_t tx_spi_buf[];

HAL_StatusTypeDef spi_receive(uint16_t size);
HAL_StatusTypeDef spi_transmit(uint8_t *input, size_t size);

#endif /* __SPI_UTILS__ */
