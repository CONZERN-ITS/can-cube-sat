/*
 * control_magnet.c
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#include "control_magnet.h"

#include "esp_log.h"

#include "shift_reg.h"
#include "init_helper.h"


static int _shift_plus;
static int _shift_minus;

static shift_reg_handler_t *_hsr;

void control_magnet_init(shift_reg_handler_t *hsr, int shift_plus, int shift_minus) {
	_shift_minus = shift_minus;
	_shift_plus = shift_plus;
	_hsr = hsr;
}
void control_magnet_enable(int bsk_number, int state) {
	if (state == 0) {
		shift_reg_set_level_pin(_hsr, _shift_plus + bsk_number * ITS_SR_PACK_SIZE, 0);
		shift_reg_set_level_pin(_hsr, _shift_minus + bsk_number * ITS_SR_PACK_SIZE, 0);
	} else if (state > 0) {
		shift_reg_set_level_pin(_hsr, _shift_plus + bsk_number * ITS_SR_PACK_SIZE, 1);
		shift_reg_set_level_pin(_hsr, _shift_minus + bsk_number * ITS_SR_PACK_SIZE, 0);
	} else if (state < 0){
		shift_reg_set_level_pin(_hsr, _shift_plus + bsk_number * ITS_SR_PACK_SIZE, 0);
		shift_reg_set_level_pin(_hsr, _shift_minus + bsk_number * ITS_SR_PACK_SIZE, 1);
	}
}

