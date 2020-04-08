/*
 * timers.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#include "timers.h"
#include "time_util.h"

#include <assert.h>


//! Текущий номер недели
static uint32_t _gps_week;


TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


static void MX_TIM2_Init(void)
{
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	assert(HAL_TIM_Base_Init(&htim2) == HAL_OK);
	assert(HAL_TIM_PWM_Init(&htim2) == HAL_OK);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR3;
	assert(HAL_TIM_SlaveConfigSynchronization(&htim2, &sSlaveConfig) == HAL_OK);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	assert(HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) == HAL_OK);

	HAL_TIM_MspPostInit(&htim2);
}


static void MX_TIM3_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	assert(HAL_TIM_Base_Init(&htim3) == HAL_OK);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	sClockSourceConfig.ClockFilter = 0;
	assert(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) == HAL_OK);
	assert(HAL_TIM_PWM_Init(&htim3) == HAL_OK);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	assert(HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig) == HAL_OK);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	assert (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) == HAL_OK);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 10;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	assert(HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);

	HAL_TIM_MspPostInit(&htim3);
}


static void MX_TIM4_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 20999;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 3;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	assert(HAL_TIM_Base_Init(&htim4) == HAL_OK);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	assert(HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) == HAL_OK);

	assert(HAL_TIM_PWM_Init(&htim4) == HAL_OK);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	assert(HAL_TIM_SlaveConfigSynchronization(&htim4, &sSlaveConfig) == HAL_OK);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	assert(HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) == HAL_OK);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	assert(HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);

	HAL_TIM_MspPostInit(&htim4);
}


//! прерывание таймера 2
void TIM2_IRQHandler()
{
	// Отдаем его халу. Впринципе нас интересует только перерывание на переполнение таймера
	// Что будет означать что прошла неделя
	HAL_TIM_IRQHandler(&htim2);
}


//! Халовский колбек на переполнение таймера
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim)
{
	// Если это был таймер2, то значит прошла неделя
	if (&htim2 == htim)
	{
		_gps_week++;
	}
}


void time_svc_timers_prepare()
{
	// Инициализируем таймеры и все сопутствующие им железочки
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();

	// Включаем PWM выводы для всех таймеров, для того, чтобы пины
	// перешли в OUTPUT режим и ставим счетчик таймера
	// в предельное значение
	// При наших настройках (PWM MODE 1, POLARITY HIGH) это выводит PWM линию в 0
	// Но при этом таймер по пуску сразу же сгенерирует UPD событие и сбросится не делая(? FIXME) лишних тиков
	__HAL_TIM_SET_COUNTER(&htim2, 0); // у TIM2 нет PWM линий и прочего

	__HAL_TIM_SET_COUNTER(&htim3, __HAL_TIM_GET_AUTORELOAD(&htim3));
	TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

	__HAL_TIM_SET_COUNTER(&htim4, __HAL_TIM_GET_AUTORELOAD(&htim4));
	TIM_CCxChannelCmd(htim4.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

	// Подождем чуть чуть, чтобы все PWM линии надежно опустились вниз
	// и все таймеры одновременно стартанули красивым передним фронтом на своих PWM линиях
	HAL_Delay(10);
}


void time_svc_timers_initial_time_preload(time_t initial_time)
{
	uint32_t tow_ms;
	uint16_t week;

	struct timeval tmv = { .tv_sec = initial_time };
	unix_time_to_gps_time(&tmv, &week, &tow_ms);

	// Убеждаемся, что нам дали миллисекунды ровно на начало секунды
	assert(tow_ms % 1000 == 0);

	// Проставляем значения
	_gps_week = week;
	__HAL_TIM_SET_COUNTER(&htim2, tow_ms);
}


void time_svc_timers_get_time(struct timeval * tmv)
{
	uint16_t week;
	uint32_t tow_ms;

	week = _gps_week;
	tow_ms = __HAL_TIM_GET_COUNTER(&htim2);

	gps_time_to_unix_time(week, tow_ms, tmv);
}
