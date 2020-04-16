/*
 * timers.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#include <assert.h>
#include <drivers/time_svc/timers_world.h>

#include "../common.h"
#include "time_util.h"


//! Текущий номер недели
static uint32_t _gps_week;


TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


static int MX_TIM2_Init(void)
{
	HAL_StatusTypeDef hal_error;
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = (SECONDS_PER_WEEK * 1000 - 1);
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hal_error = HAL_TIM_Base_Init(&htim2);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	hal_error = HAL_TIM_PWM_Init(&htim2);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR3;
	hal_error = HAL_TIM_SlaveConfigSynchronization(&htim2, &sSlaveConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	hal_error = HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	HAL_TIM_MspPostInit(&htim2);
	return 0;
}


static int MX_TIM3_Init(void)
{
	HAL_StatusTypeDef hal_error;
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hal_error = HAL_TIM_Base_Init(&htim3);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	sClockSourceConfig.ClockFilter = 0;
	hal_error = HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	hal_error = HAL_TIM_PWM_Init(&htim3);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	hal_error = HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	hal_error = HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 10;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	hal_error = HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	HAL_TIM_MspPostInit(&htim3);
	return 0;
}


static int MX_TIM4_Init(void)
{
	HAL_StatusTypeDef hal_error;

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 20999;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 3;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hal_error = HAL_TIM_Base_Init(&htim4);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	hal_error = HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	hal_error = HAL_TIM_PWM_Init(&htim4);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	hal_error = HAL_TIM_SlaveConfigSynchronization(&htim4, &sSlaveConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	hal_error = HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	hal_error = HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	HAL_TIM_MspPostInit(&htim4);
	return 0;
}


//! прерывание таймера 2
void TIM2_IRQHandler()
{
	// Прошла неделя!
	_gps_week++;

	// Дальше дадим халу, чтобы он снял все флаги
	HAL_TIM_IRQHandler(&htim2);
}


int time_svc_world_timers_prepare()
{
	int rc;
	// Инициализируем таймеры и все сопутствующие им железочки
	rc = MX_TIM2_Init();
	if (0 != rc) return rc;

	rc = MX_TIM3_Init();
	if (0 != rc) return rc;

	rc = MX_TIM4_Init();
	if (0 != rc) return rc;

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
	return 0;
}


void time_svc_world_timers_set_time(time_t initial_time)
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


void time_svc_world_timers_start()
{
	// Запускаем все таймеры!
	// именно в таком порядке
	__HAL_TIM_ENABLE(&htim2);
	__HAL_TIM_ENABLE(&htim3);
	__HAL_TIM_ENABLE(&htim4);

	// Для счёта недель.
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE); // почему-то по старту он стоит
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
}


void time_svc_world_timers_get_time(struct timeval * tmv)
{
	uint16_t week, week2;
	uint32_t tow_ms;

	week = _gps_week;
	tow_ms = __HAL_TIM_GET_COUNTER(&htim2);
	week2 = _gps_week;

	if (week != week2 && (tow_ms & 0xFFFFFF00) == 0)
	{
		// Это значит, что пока мы читали week - таймер успел переполнится
		// И поскольку tow_ms не перевалило еще даже за 256, то мы прочли значение таймера
		// уже после переполнения

		// Значит и неделю нужно брать следующую
		week = week2;
	}

	gps_time_to_unix_time(week, tow_ms, tmv);
}
