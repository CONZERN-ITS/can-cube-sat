/*
 * control_magnet.h
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_CONTROL_MAGNET_INC_CONTROL_MAGNET_H_
#define COMPONENTS_CONTROL_MAGNET_INC_CONTROL_MAGNET_H_

#include "shift_reg.h"

void control_magnet_init(shift_reg_handler_t *hsr, int shift_plus, int shift_minus);
void control_magnet_enable(int bsk_number, int state);


#endif /* COMPONENTS_CONTROL_MAGNET_INC_CONTROL_MAGNET_H_ */
