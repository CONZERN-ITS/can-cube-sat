/*
 * app_main.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */


#include "app_main.h"

#include <stdio.h>
#include <inttypes.h>

#include <stm32f1xx_hal.h>

#include <mavlink/its/mavlink.h>

#include "main.h"

#include <its-time.h>

#include "util.h"
#include "analog.h"
#include "bme.h"
#include "me2o2.h"
#include "mics6814.h"
#include "integrated_temperature.h"


its_pld_status_t pld_g_status = {0};


extern ADC_HandleTypeDef hadc1;


int app_main()
{
	int rc;

	its_time_init();

	rc = its_pld_bme280_init();
	pld_g_status.bme_init_error = rc;

	rc = its_pld_analog_init();
	pld_g_status.adc_init_error = rc;

	rc = me2o2_init();
	pld_g_status.me2o2_init_error = rc;

	rc = its_pld_inttemp_init();
	pld_g_status.inttemp_init_error = rc;

	rc = mics6814_init();
	pld_g_status.mics6814_errors_counter = rc;

	while(1)
	{
		printf("======================================================\n");

		mavlink_pld_bme280_data_t msg = {0};
		rc = its_pld_bme280_read(&msg);
		pld_g_status.bme_last_error = rc;
		pld_g_status.bme_error_counter += (0 == rc) ? 1 : 0;
		printf("bme: e=%ld, t=%fC, p=%fpa, hum=%f%%, alt=%fm\n",
				pld_g_status.bme_last_error, msg.temperature, msg.pressure, msg.humidity, msg.altitude);
		printf("time = %"PRIu64", %"PRIu32"\n", msg.time_s, msg.time_us);

		mavlink_pld_me2o2_data_t me2o2_msg = {0};
		rc = me2o2_read(&me2o2_msg);
		printf("me2o2: o2=%f%%\n", me2o2_msg.o2_conc);
		printf("time = %"PRIu64", %"PRIu32"\n", me2o2_msg.time_s, me2o2_msg.time_us);

		mavlink_pld_mics_6814_data_t mics_msg = {0};
		mics6814_read(&mics_msg);
		printf("mics6814: co=%fppm, nh3=%fppm, no2=%fppm\n", mics_msg.co_conc, mics_msg.nh3_conc, mics_msg.no2_conc);
		printf("time = %"PRIu64", %"PRIu32"\n", mics_msg.time_s, mics_msg.time_us);

		mavlink_own_temp_t own_temp_msg;
		its_pld_inttemp_read(&own_temp_msg);
		printf("otemp: %fC\n", own_temp_msg.deg);
		printf("time = %"PRIu64", %"PRIu32"\n", own_temp_msg.time_s, own_temp_msg.time_us);

		HAL_Delay(2000);
	}

	return 0;
}
