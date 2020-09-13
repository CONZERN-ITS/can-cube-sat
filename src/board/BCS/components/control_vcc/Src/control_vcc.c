/*
 * control_vcc.c
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */


#include "control_vcc.h"

#include "esp_log.h"

#include "shift_reg.h"
#include "init_helper.h"



static int _shift;

static shift_reg_handler_t *_hsr;

static int _pl_pin;

void control_vcc_init(shift_reg_handler_t *hsr, int shift, uint32_t pl_pin) {
	_shift = shift;
	_hsr = hsr;
	_pl_pin = pl_pin;
	for (int i = 0; i < ITS_BSK_COUNT; i++) {
		shift_reg_set_level_pin(_hsr, _shift + i * ITS_SR_PACK_SIZE, 1);
	}
}
void control_vcc_bsk_enable(int bsk_number, int is_on) {
	shift_reg_set_level_pin(_hsr, _shift + bsk_number * ITS_SR_PACK_SIZE, is_on > 0);
}

void control_vcc_pl_enable(int is_on) {
	gpio_set_level(_pl_pin, is_on != 0);
}
