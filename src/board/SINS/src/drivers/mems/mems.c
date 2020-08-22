/*
 * mems.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */


#include "mems.h"

#include <assert.h>

#include "../common.h"


I2C_HandleTypeDef hmems_i2c;


int mems_init_bus()
{
	//	I2C init
	hmems_i2c.Instance = I2C2;
	hmems_i2c.Mode = HAL_I2C_MODE_MASTER;

	hmems_i2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hmems_i2c.Init.ClockSpeed = 400000;
	hmems_i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hmems_i2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hmems_i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hmems_i2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	hmems_i2c.Init.OwnAddress1 = 0x00;

	HAL_StatusTypeDef hal_status =  HAL_I2C_Init(&hmems_i2c);
	return sins_hal_status_to_errno(hal_status);
}


void mems_swrst(void)
{
	SET_BIT(hmems_i2c->Instance->CR1, I2C_CR1_SWRST);
}


void mems_generate_stop_flag(void)
{
	SET_BIT(hmems_i2c->Instance->CR1,I2C_CR1_STOP);
}


int mems_get_gyro_staticShift(float* gyro_staticShift)
{
	int error = 0;
	uint16_t zero_orientCnt = 2000;

	//	Get static gyro shift
	float gyro[3] = {0, 0, 0};
	for (int i = 0; i < zero_orientCnt; i++)
	{
		//	Collect data
		error = mems_lsm6ds3_get_g_data_rps(gyro);
		if (0 != error)
			return error;

		for (int m = 0; m < 3; m++)
			gyro_staticShift[m] += gyro[m];
	}


	for (int m = 0; m < 3; m++) {
		gyro_staticShift[m] /= zero_orientCnt;
	}


	return 0;
}


int mems_get_accel_staticShift(float* accel_staticShift)
{
	int error = 0;
	uint16_t zero_orientCnt = 300;


	float accel[3] = {0, 0, 0};
	for (int i = 0; i < zero_orientCnt; i++)
	{
		//	Collect data
		error = mems_lsm6ds3_get_xl_data_g(accel);
		if (0 != error)
			return error;

		//	Set accel static shift vector as (0,0,g)
		accel_staticShift[0] = 0;
		accel_staticShift[1] = 0;
		accel_staticShift[2] += sqrt(accel[0]*accel[0] + accel[1]*accel[1] + accel[2]*accel[2]);
	}

	//	Divide shift by counter
	for (int m = 0; m < 3; m++)
		accel_staticShift[m] /= zero_orientCnt;


	return 0;
}
