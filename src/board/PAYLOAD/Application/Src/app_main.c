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

	return 0;
}
