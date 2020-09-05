/*
 * control_vcc.h
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_CONTROL_VCC_INC_CONTROL_VCC_H_
#define COMPONENTS_CONTROL_VCC_INC_CONTROL_VCC_H_

#include "shift_reg.h"

void control_vcc_init(shift_reg_handler_t *hsr, int shift, uint32_t pl_pin);

void control_vcc_bsk_enable(int bsk_number, int is_on);

void control_vcc_pl_enable(int is_on);

#endif /* COMPONENTS_CONTROL_VCC_INC_CONTROL_VCC_H_ */
