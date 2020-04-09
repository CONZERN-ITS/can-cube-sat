/*
 * time_svc.h
 *
 *  Created on: 7 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_TIME_SVC_H_
#define DRIVERS_TIME_SVC_TIME_SVC_H_

#include <stdint.h>

#include <time.h>


//! Запуск службы мирового времени
/*! Эта служба времени запускается от RTC и затем корректируется по GPS */
int time_svc_world_init(void);

//! Получение мирового времени в эпохе UNIX для UTC+0 зоны
/*! Возвращает стандартный struct timeval как gettimeoftheday */
void time_svc_get_world_time(struct timeval * tv);


//! Запуск стабильной службы времени
int time_svc_steady_init(void);

//! Получение стабильного времени
/*! Возвращает микросекунды от запуска службы */
uint64_t time_svc_get_steady_time(void);


#endif /* DRIVERS_TIME_SVC_TIME_SVC_H_ */
