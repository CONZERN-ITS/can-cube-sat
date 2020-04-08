/*
 * timers.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_TIMERS_H_
#define DRIVERS_TIME_SVC_TIMERS_H_


#include <stm32f4xx_hal.h>


//! Дескриптор таймера 2. Этот таймер считает миллисекунды недели
extern TIM_HandleTypeDef htim2;
//! Дескриптор таймера 3. Этот таймер обеспечивает PPS для прочих устройств
extern TIM_HandleTypeDef htim3;
//! Дескриптор таймера 4. Этот таймер обеспечивает миллисекундный клок остальным таймерами
/*! И отвечает за подсчет субмилисекунд */
extern TIM_HandleTypeDef htim4;


//! Инициализиует таймеры службы времени
/*! И все сопутствующие им железочки.
 *  Активируются все выходные линии таймеров и укладываются на 0
 *  После вызова этой функции следует чуточку подождать, чтобы все выходные линии надежно упали в 0
 *  и по пуску таймеров на выходе из чипа были красивые ровные фронты */
void time_svc_timers_prepare(void);

//! Пуск таймеров службы времени!
void time_svc_timers_start(void);


#endif /* DRIVERS_TIME_SVC_TIMERS_H_ */
