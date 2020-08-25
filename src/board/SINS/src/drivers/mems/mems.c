/*
 * mems.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */


#include "mems.h"

#include <assert.h>

#include "../common.h"


#define HMEMS_I2C_INSTANCE	I2C2
#define HMEMS_I2C_FORCE_RESET 	__HAL_RCC_I2C2_FORCE_RESET
#define HMEMS_I2C_RELEASE_RESET __HAL_RCC_I2C2_RELEASE_RESET


I2C_HandleTypeDef hmems_i2c = {
		.Instance = HMEMS_I2C_INSTANCE,
		.Mode = HAL_I2C_MODE_MASTER,
		.Init = {
				.AddressingMode = I2C_ADDRESSINGMODE_7BIT,
				.ClockSpeed = 400000,
				.DualAddressMode = I2C_DUALADDRESS_DISABLE,
				.DutyCycle = I2C_DUTYCYCLE_2,
				.GeneralCallMode = I2C_GENERALCALL_DISABLE,
				.NoStretchMode = I2C_NOSTRETCH_DISABLE,
				.OwnAddress1 = 0x00
		}
};


int mems_init_bus()
{
	//	SET_BIT(hmems_i2c.Instance->CR2, I2C_CR1_SWRST);
	//	CLEAR_BIT(hmems_i2c.Instance->CR1, I2C_CR1_SWRST);

		HMEMS_I2C_FORCE_RESET();
		HMEMS_I2C_RELEASE_RESET();

		//HAL_I2C_DeInit(&hmems_i2c);
		__HAL_I2C_RESET_HANDLE_STATE(&hmems_i2c);

		HAL_StatusTypeDef hal_status =  HAL_I2C_Init(&hmems_i2c);
		return sins_hal_status_to_errno(hal_status);
}


void mems_generate_stop_flag(void)
{
	SET_BIT(hmems_i2c.Instance->CR1,I2C_CR1_STOP);
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
