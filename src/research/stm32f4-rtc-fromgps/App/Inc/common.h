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

//! Дельта времени между двумя значениями таков хала с учетом возможного переполнения
uint64_t sins_hal_tick_diff(uint32_t start, uint32_t stop);

#endif /* DRIVERS_COMMON_H_ */
