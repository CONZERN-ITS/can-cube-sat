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

void control_vcc_init(shift_reg_handler_t *hsr, int shift) {
	_shift = shift;
	_hsr = hsr;
	for (int i = 0; i < ITS_BSK_COUNT; i++) {
		shift_reg_set_level_pin(_hsr, _shift + ITS_BSK_COUNT * ITS_SR_PACK_SIZE, 1);
	}
}
void control_vcc_bsk_enable(int bsk_number, int is_on) {
	shift_reg_set_level_pin(_hsr, _shift + bsk_number * ITS_SR_PACK_SIZE, 1);
}
