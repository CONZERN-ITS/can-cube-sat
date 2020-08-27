/*
 * mems.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_MEMS_MEMS_H_
#define DRIVERS_MEMS_MEMS_H_


#include <stm32f4xx_hal.h>

#include "lsm6ds3.h"
#include "lis3mdl.h"


//! Дескриптор шины на которой сидят MEMS датчики
extern I2C_HandleTypeDef hmems_i2c;


//! Инициализация шины для работы с датчиками
int mems_init_bus(void);


// делает software reset
void mems_swrst(void);

void scl_clocking(int count_clocking);


//генерирует стоп флаг
void mems_generate_stop_flag(void);

/**
  * @brief	Static function used to get accel static shift
  * @param	gyro_staticShift	Array used to get gyro shift from
  * @param	accel_staticShift	Array used to store accel shift
  * @retval	Device's wire error */
int mems_get_accel_staticShift(float* accel_staticShift);


/**
  * @brief	Static function used to get gyro static shift
  * @param	gyro_staticShift	Array used to store that shift
  * @retval	Device's wire error */
int mems_get_gyro_staticShift(float* gyro_staticShift);


#endif /* DRIVERS_MEMS_MEMS_H_ */
