/*
 * time_svc.c
 *
 *  Created on: 7 апр. 2020 г.
 *      Author: snork
 */

#include "time_svc.h"

#include <stm32f4xx_hal.h>
#include <assert.h>

#include "rtc.h"
#include "timers.h"


static uint32_t _gps_week;


void TIM2_IRQHandler()
{
	HAL_TIM_IRQHandler(&htim2);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (&htim2 == htim)
	{
		_gps_week++;
	}
}


void time_svc_init(void)
{
}
