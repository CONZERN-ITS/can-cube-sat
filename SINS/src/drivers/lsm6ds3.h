/*
 * lsm6ds3.h
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#ifndef DRIVERS_LSM6DS3_H_
#define DRIVERS_LSM6DS3_H_

#include "lsm6ds3_reg.h"

int32_t lsm6ds3_init(void);

uint32_t lsm6ds3_get_xl_data_g(float* accel);
uint32_t lsm6ds3_get_g_data_rps(float* gyro);


#endif /* DRIVERS_LSM6DS3_H_ */
