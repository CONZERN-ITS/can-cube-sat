/*
 * time_util.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_TIME_UTIL_H_
#define DRIVERS_TIME_SVC_TIME_UTIL_H_


#include <stdint.h>
#include <time.h>

#include <stm32f4xx_hal.h>

//! Перевод времени GPS в UNIX время
struct timeval gps_time_to_unix_time(uint16_t week, uint32_t tow_ms);

#endif /* DRIVERS_TIME_SVC_TIME_UTIL_H_ */
