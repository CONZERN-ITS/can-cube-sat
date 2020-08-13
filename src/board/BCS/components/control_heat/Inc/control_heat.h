/*
 * control_heat.h
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_CONTROL_HEAT_INC_CONTROL_HEAT_H_
#define COMPONENTS_CONTROL_HEAT_INC_CONTROL_HEAT_H_

#include "shift_reg.h"

#define CONTROL_HEAT_HIGHTHD 10.0
#define CONTROL_HEAT_LOWTHD -5.0

void control_heat_init(shift_reg_handler_t *hsr, int shift);

#endif /* COMPONENTS_CONTROL_HEAT_INC_CONTROL_HEAT_H_ */
