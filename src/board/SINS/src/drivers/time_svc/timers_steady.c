/*
 * timers_steady.c
 *
 *  Created on: 9 апр. 2020 г.
 *      Author: snork
 */

#include "timers_steady.h"

#include <stm32f4xx_hal.h>

#include "../common.h"


static TIM_HandleTypeDef htim6;


//! Счетчик миллисекунд
static uint32_t _ovf_counter = 0;


static int _prepare_timer()
{
	HAL_StatusTypeDef hal_error;
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 83; // запустим его с периодом в 1мкс
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 0xFFFF-1; // будем крутить его до отсечки
	hal_error = HAL_TIM_Base_Init(&htim6);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	hal_error = HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);


	// nvic в hal_msp
	return 0;
}


void TIM6_DAC_IRQHandler(void)
{
	// таймер переполнился
	_ovf_counter++;

	// Дальше дадим халу, чтобы он снял все флаги
	HAL_TIM_IRQHandler(&htim6);
}



int time_svc_steady_timers_start()
{
	int rc;

	rc = _prepare_timer();
	if (0 != rc)
		return rc;

	_ovf_counter = 0;
	__HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE(&htim6);

	return 0;
}


uint64_t time_svc_steady_timers_get_time()
{
	uint32_t ovf, ovf2;
	uint16_t mcs;

	ovf = _ovf_counter;
	mcs = __HAL_TIM_GET_COUNTER(&htim6);
	ovf2 = _ovf_counter;

	if (ovf != ovf2 && (mcs & 0xFF00) == 0)
	{
		// Это значит что после того как мы прочитали переполнения
		// таймер переполнился и мы взяли значение счетчика для следующей итерации таймера
		// поэтому берем правильное значение переполнений
		ovf = ovf2;
	}

	return ((uint64_t)ovf << (4*8)) | mcs;
}
