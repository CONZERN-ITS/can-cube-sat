/*
 * common.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_COMMON_H_
#define DRIVERS_COMMON_H_

#include <stm32f4xx_hal.h>


//! Ошибка в терминах ХАЛа в errno
int sins_hal_status_to_errno(HAL_StatusTypeDef h_status);

#endif /* DRIVERS_COMMON_H_ */
