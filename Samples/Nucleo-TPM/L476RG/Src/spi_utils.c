#include "spi_utils.h"


SPI_HandleTypeDef hspi2;

uint8_t rx_spi_buf[512] = {0};
uint8_t tx_spi_buf[512] = {0};

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_SLAVE;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

int receive() {
  int ret = -1;
  HAL_StatusTypeDef hstatus = HAL_SPI_Receive(&hspi2, rx_spi_buf, 1, 500);
  if (hstatus == HAL_OK) {
    ret = rx_spi_buf[0];
  }
  else {
//    printf("failed\n");
  }

  return ret;
}

int transmit(uint8_t *input, size_t size) {
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
