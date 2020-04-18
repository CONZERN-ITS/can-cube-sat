/*
 * pin_init.h
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef MAIN_INIT_HELPER_H_
#define MAIN_INIT_HELPER_H_

#include "driver/i2c.h"
#include "driver/gpio.h"

#define ITS_IMI_DEVICE_COUNT 		1
#define ITS_ARK_1_ADDRESS 	0x68

#define ITS_I2CTM_FREQ 		50000
#define ITS_I2CTM_PORT 		I2C_NUM_0


void init_helper(void);


#endif /* MAIN_INIT_HELPER_H_ */
