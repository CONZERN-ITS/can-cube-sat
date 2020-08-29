/*
 * lsm6ds3.h
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#ifndef DRIVERS_LSM6DS3_H_
#define DRIVERS_LSM6DS3_H_

#include "lsm6ds3_reg.h"


int mems_lsm6ds3_init(void);

int mems_lsm6ds3_get_xl_data_g(int16_t * data, float * accel);
int mems_lsm6ds3_get_xl_data_raw(int16_t * data);

int mems_lsm6ds3_get_g_data_rps(int16_t * data, float * gyro);
int mems_lsm6ds3_get_g_data_raw(int16_t * data);


#endif /* DRIVERS_LSM6DS3_H_ */
