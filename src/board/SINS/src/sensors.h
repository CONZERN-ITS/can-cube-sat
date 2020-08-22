/*
 * sensors.h
 *
 *  Created on: 22 авг. 2020 г.
 *      Author: developer
 */

#ifndef SENSORS_H_
#define SENSORS_H_

typedef struct mems_state_t
{
	int bus_init_required;
	int bus_error;

	int lsm6ds3_init_required;
	int lsm6ds3_error;

	int lis3mdl_init_required;
	int lis3mdl_error;
} mems_state_t;


//инициализация всех mems датчиков вместе с шиной
int sensors_init(void);


// чтение  даннных сгироскопа и акселерометра и гироскопа
int sensors_lsm6ds3_read(float * accel, float * gyro);


// чтение данных с магнитометра
int sensors_lis3mdl_read(float * magn);


#endif /* SENSORS_H_ */
