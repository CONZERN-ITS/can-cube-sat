/*
 * adc.c
 *
 *  Created on: May 30, 2020
 *      Author: snork
 */

#include "analog.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <stm32f4xx_hal.h>

#include "drivers/common.h"


ADC_HandleTypeDef hadc1;

#define _ADC_HAL_TIMEOUT (HAL_MAX_DELAY)

#define _ADC_HANDLE (&hadc1)


int analog_init(void)
{
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	*/
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	int error = HAL_ADC_Init(&hadc1);

	// Включаем АЦП
	__HAL_ADC_ENABLE(&hadc1);


	return error;
}


static int _channgel_config_for_target(analog_target_t target, ADC_ChannelConfTypeDef * config)
{
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/

	config->Channel = ADC_CHANNEL_TEMPSENSOR;
	config->Rank = 1;
	config->SamplingTime = ADC_SAMPLETIME_3CYCLES;
	int error = HAL_ADC_ConfigChannel(&hadc1, config);

	return error;
}


int analog_restart(void)
{
	// Глушим ADC к чертям
	HAL_ADC_DeInit(&hadc1);
	__HAL_RCC_ADC_FORCE_RESET();
	__HAL_RCC_ADC_RELEASE_RESET();
	__HAL_ADC_RESET_HANDLE_STATE(&hadc1);

	// Включаем

	HAL_StatusTypeDef hal_rc;
	hal_rc = HAL_ADC_Init(&hadc1);
	int rc = sins_hal_status_to_errno(hal_rc);
	if (0 != rc)
		return rc;

	return analog_init();
}


int analog_get_raw(analog_target_t target, uint16_t * value)
{
	int error = 0;

	ADC_ChannelConfTypeDef config;
	error = _channgel_config_for_target(target, &config);
	if (0 != error)
		return error;

	HAL_StatusTypeDef hal_error = HAL_ADC_ConfigChannel(_ADC_HANDLE, &config);
	error = sins_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	hal_error = HAL_ADC_Start(&hadc1);
	error = sins_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	hal_error = HAL_ADC_PollForConversion(_ADC_HANDLE, _ADC_HAL_TIMEOUT);
	error = sins_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	*value = (uint16_t)HAL_ADC_GetValue(_ADC_HANDLE);
	return 0;
}
