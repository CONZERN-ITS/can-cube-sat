/*
 * time_svc.h
 *
 *  Created on: Jun 12, 2020
 *      Author: snork
 */

#ifndef INC_TIME_SVC_H_
#define INC_TIME_SVC_H_

#include <sys/time.h>

#include <stm32f1xx_hal.h>

#include "mavlink_main.h"

extern TIM_HandleTypeDef htim4;
// Халовский хендл таймера на котором мы работаем
#define TIMESVC_TIM_HANDLE (&htim4)


//! Инициализация службы времени
/*! таймер, указанный TIMESVC_TIM_HANDLE должен быть настроен так, чтобы частота
 *  у него была 2000 кГц а период 2000. То есть один тик весит пол
 *  миллисекунды а переполняется таймер раз в секунду */
int time_svc_init(void);

//! Получение текущего времени
int time_svc_gettimeofday(struct timeval * tmv);

//! Установка текущего времени
int time_svc_settimeofday(const struct timeval * tmv);


//! Эту функцию нужно вызывать в обработчике прерывания целевого таймера
void time_svc_on_tim_interrupt(void);

//! Эту функцию нужно вызывать в обработчике прерывания по получению временного синхросигнала
void time_svc_on_sync_interrupt(void);

//! Эту функцию нужно вызывать при получении mavlink сообщения от ведущего
void time_svc_on_mav_message(const mavlink_message_t * msg);

#endif /* INC_TIME_SVC_H_ */
