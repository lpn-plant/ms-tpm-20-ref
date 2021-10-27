/*
 * cdc_get_data.h
 *
 *  Created on: Oct 27, 2021
 *      Author: acwiek
 */

#ifndef CDC_DATA_H_
#define CDC_DATA_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
  size_t avail;
  uint8_t *data;
} cdc_input_data;

void cdc_put_data(uint8_t *buffer, uint32_t size);
void cdc_get_data(cdc_input_data *input);
void cdc_buffer_init();

#endif /* CDC_DATA_H_ */
