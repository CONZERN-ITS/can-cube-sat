/*
 * app_main.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */


#include "app_main.h"

#include <stdio.h>

#include <stm32f1xx_hal.h>

#include <mavlink/its/mavlink.h>

#include "main.h"

#include <its-time.h>

#include "util.h"
#include "analog.h"
#include "bme.h"


its_pld_status_t pld_g_status = {0};


extern ADC_HandleTypeDef hadc1;


int app_main()
{
	int rc;

	its_time_init();

	rc = its_pld_analog_init();
	pld_g_status.adc_init_error = rc;

	rc = its_pld_bme280_init();
	pld_g_status.bme_init_error = rc;

	while(1)
	{
		mavlink_pld_bme280_data_t msg = {0};
		rc = its_pld_bme280_read(&msg);
		pld_g_status.bme_last_error = rc;
		pld_g_status.bme_error_counter += (0 == rc) ? 1 : 0;

		printf("%f, %f, %f, %f\n", msg.temperature, msg.pressure, msg.humidity, msg.altitude);
	}

	return 0;
}
