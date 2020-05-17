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
	int32_t adc_last_error;
	uint16_t adc_errors_counter;
} its_pld_status_t;


extern its_pld_status_t its_pld_status;

#endif /* INC_APP_MAIN_H_ */
