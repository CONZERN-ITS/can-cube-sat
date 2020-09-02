/*
 * shift_reg.c
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#include "shift_reg.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

esp_err_t shift_reg_load(shift_reg_handler_t *hsr) {
	spi_transaction_t trans = {0};
	trans.tx_buffer = hsr->byte_arr;
	trans.length = hsr->arr_size * 8; //В битах

	if (xSemaphoreTake(hsr->mutex, hsr->ticksToWait) == pdFALSE) {
		return -1;
	}
	esp_err_t ret = spi_device_polling_transmit(hsr->hspi, &trans);

	if (xSemaphoreGive(hsr->mutex) == pdFALSE) {
		return -1;
	}
	return ret;
}

esp_err_t shift_reg_init_spi(shift_reg_handler_t *hsr, spi_host_device_t port,
		int bit_count, TickType_t ticksToWait, int pin_cs) {

	esp_err_t ret;

	spi_device_interface_config_t devcfg={
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,

		.clock_speed_hz = 500000,
		.mode = 0,
		.spics_io_num = pin_cs,
		.queue_size = 30,
		.pre_cb = 0,
	};
	ret = spi_bus_add_device(port, &devcfg, &hsr->hspi);
	if (ret != ESP_OK) {
		return ret;
	}
	int byte_count = (bit_count + 7) / 8;
	if (byte_count > SHIFT_REG_MAX_BYTES) {
		return ESP_ERR_INVALID_ARG;
	}
	hsr->arr_size = byte_count;

	hsr->mutex = xSemaphoreCreateMutex();
	return 0;
}

void shift_reg_toggle_pin(shift_reg_handler_t *hsr, int pin) {
	assert(pin < hsr->arr_size * 8);
	//Обратный порядок байт
	hsr->byte_arr[hsr->arr_size - 1 - pin / 8] ^= (1 << (pin % 8));
}
void shift_reg_set_level_pin(shift_reg_handler_t *hsr, int pin, int level) {
	assert(pin < hsr->arr_size * 8);
	hsr->byte_arr[hsr->arr_size - 1 - pin / 8] = (hsr->byte_arr[hsr->arr_size - 1 - pin / 8] & ~(1 << (pin % 8))) | (level << (pin % 8));
}
