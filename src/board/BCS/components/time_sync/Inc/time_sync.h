/*
 * time_sync.h
 *
 *  Created on: Jul 14, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_TIME_SYNC_INC_TIME_SYNC_H_
#define COMPONENTS_TIME_SYNC_INC_TIME_SYNC_H_


#include "driver/gpio.h"
#include "hal/i2c_types.h"
#include "soc/i2c_caps.h"

typedef struct {
	gpio_num_t pin;
	struct timeval here;
	int is_updated;
} ts_sync;

void time_sync_sins_install(ts_sync *cfg);



#endif /* COMPONENTS_TIME_SYNC_INC_TIME_SYNC_H_ */
