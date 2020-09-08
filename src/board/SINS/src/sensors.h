/*
 * sensors.h
 *
 *  Created on: 22 авг. 2020 г.
 *      Author: developer
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <stdint.h>

typedef struct mems_state_t
{
	int bus_ready;
	int bus_error_counter;
	int bus_error;

	int lsm6ds3_ready;
	int lsm6ds3_error_counter;
	int lsm6ds3_error;
	int lsm6ds3_accel_data_counter;
	int lsm6ds3_gyro_data_counter;

	int lis3mdl_ready;
	int lis3mdl_error_counter;
	int lis3mdl_error;
	int lis3mdl_magn_data_counter;

	int16_t accel_prew[3];
	int16_t gyro_prew[3];
	int16_t magn_prew[3];

} mems_state_t;


//инициализация всех mems датчиков вместе с шиной
int sensors_init(void);


// чтение  даннных сгироскопа и акселерометра и гироскопа
int sensors_lsm6ds3_read(float * accel, float * gyro);


// чтение данных с магнитометра
int sensors_lis3mdl_read(float * magn);


extern mems_state_t mems_state;

#endif /* SENSORS_H_ */
