/*
 * app_main.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */


#include "app_main.h"


#include <stm32f1xx_hal.h>
#include "main.h"

#include "util.h"

its_pld_status_t its_pld_status = {0};


extern ADC_HandleTypeDef hadc1;


int app_main()
{
	// Калибруем ацп
	int error = its_pld_hal_status_to_errno(HAL_ADCEx_Calibration_Start(&hadc1));
	if (0 != error)
	{
		its_pld_status.adc_errors_counter++;
		its_pld_status.adc_last_error = error;

		// Однако. даже если что-то пошло не так - продолжаем работать
	}
	__HAL_ADC_ENABLE(&hadc1);

	HAL_ADC_PollForConversion(hadc, Timeout)


	ADC_ChannelConfTypeDef adc_cc;
	adc_cc.Channel = ADC_CHANNEL_0;
	adc_cc.Rank = ADC_REGULAR_RANK_1;
	adc_cc.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
	HAL_ADC_ConfigChannel(&hadc1, &adc_cc);
}
