/*
 * app_main.h
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */

#ifndef INC_APP_MAIN_H_
#define INC_APP_MAIN_H_

#include <stdint.h>


typedef struct its_pld_status_t
{
	int32_t bme_init_error;
	int32_t bme_last_error;
	uint16_t bme_error_counter;

	int32_t adc_init_error;
	int32_t adc_last_error;
	uint16_t adc_errors_counter;

	int32_t me2o2_init_error;
	int32_t me2o2_last_error;
	uint16_t me2o2_errors_counter;
} its_pld_status_t;


extern its_pld_status_t pld_g_status;

#endif /* INC_APP_MAIN_H_ */
