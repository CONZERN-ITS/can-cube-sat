/*
 * adc.c
 *
 *  Created on: May 30, 2020
 *      Author: snork
 */

#include "sensors/analog.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <stm32f1xx_hal.h>

#include "util.h"


extern ADC_HandleTypeDef hadc1;


#define _ITS_PLD_ADC_HAL_TIMEOUT (HAL_MAX_DELAY)

#define _ITS_PLD_ADC_HANDLE (&hadc1)



//! Создает структуру конфигурации канала АЦП
static int _channgel_config_for_target(its_pld_analog_target_t target, ADC_ChannelConfTypeDef * config)
{
	// Все согласно разводке на плате
	int error = 0;
	switch(target)
	{
	case ITS_PLD_ANALOG_TARGET_MICS6814_NO2:
		config->Channel = ADC_CHANNEL_0;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_NH3:
		config->Channel = ADC_CHANNEL_1;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_CO:
		config->Channel = ADC_CHANNEL_2;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_ME202_O2:
		config->Channel = ADC_CHANNEL_3;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_INTEGRATED_TEMP:
		config->Channel = ADC_CHANNEL_TEMPSENSOR;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	default:
		error = -ENOSYS;
		break;
	}

	return error;
}


int its_pld_analog_init()
{
	// Предположим, что куб все правильно настроил
	// Нам не нужно никакой автоматики, настриваемся на один канал
	// Вроде вот такого
	//	hadc1.Instance = ADC1;
	//	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	//	hadc1.Init.ContinuousConvMode = DISABLE;
	//	hadc1.Init.DiscontinuousConvMode = DISABLE;
	//	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	//	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	//	hadc1.Init.NbrOfConversion = 1;
	//	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	//	{
	//	Error_Handler();
	//	}

	// Калибруем ацп
	int error = its_pld_hal_status_to_errno(HAL_ADCEx_Calibration_Start(&hadc1));
	// Ошибку тут проигнорируем. Вдруг как-то да заработает дальше

	// Включаем АЦП
	__HAL_ADC_ENABLE(&hadc1);

	return error;
}


int its_pld_analog_get_raw(its_pld_analog_target_t target, uint16_t * value)
{
	int error = 0;

	ADC_ChannelConfTypeDef config;
	error = _channgel_config_for_target(target, &config);
	if (0 != error)
		return error;

	HAL_StatusTypeDef hal_error = HAL_ADC_ConfigChannel(_ITS_PLD_ADC_HANDLE, &config);
	error = its_pld_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	hal_error = HAL_ADC_Start(&hadc1);
	error = its_pld_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	hal_error = HAL_ADC_PollForConversion(_ITS_PLD_ADC_HANDLE, _ITS_PLD_ADC_HAL_TIMEOUT);
	error = its_pld_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	*value = (uint16_t)HAL_ADC_GetValue(_ITS_PLD_ADC_HANDLE);
	return 0;
}
