/*
 * shift_reg.h
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_SHIFT_REG_INC_SHIFT_REG_H_
#define COMPONENTS_SHIFT_REG_INC_SHIFT_REG_H_

#include <stdint.h>
#include "driver/spi_master.h"

#define SHIFT_REG_MAX_BYTES 3

typedef struct {
	spi_device_handle_t hspi;
	uint8_t byte_arr[SHIFT_REG_MAX_BYTES];
	int arr_size;
	TickType_t ticksToWait;
} shift_reg_handler_t;


esp_err_t shift_reg_init_spi(shift_reg_handler_t *hsr, spi_host_device_t port,
		int bit_count, TickType_t ticksToWait, int pin_cs);
esp_err_t shift_reg_load(shift_reg_handler_t *hsr);

void shift_reg_toggle_pin(shift_reg_handler_t *hsr, int pin);
void shift_reg_set_level_pin(shift_reg_handler_t *hsr, int pin, int level);

#endif /* COMPONENTS_SHIFT_REG_INC_SHIFT_REG_H_ */
