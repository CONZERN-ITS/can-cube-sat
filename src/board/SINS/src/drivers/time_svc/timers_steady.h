/*
 * timers_steady.h
 *
 *  Created on: 9 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_TIMERS_STEADY_H_
#define DRIVERS_TIME_SVC_TIMERS_STEADY_H_

#include <stdint.h>

//! Пуск таймера стабильного времени службы времени!
int time_svc_steady_timers_start(void);

//! Получение текущего стабильного времени службы времени
/*! Возвращает микросекунды от запуска таймера */
uint64_t time_svc_steady_timers_get_time(void);


#endif /* DRIVERS_TIME_SVC_TIMERS_STEADY_H_ */
