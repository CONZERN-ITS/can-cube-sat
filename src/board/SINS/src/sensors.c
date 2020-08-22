/*
 * sensors.c
 *
 *  Created on: 22 авг. 2020 г.
 *      Author: developer
 */

#include "sensors.h"
#include "drivers/mems/mems.h"

i2c_error_codes i2c_errors;

mems_state_t state = {
	.mems_bus_init_required = 1,
	.lsm6ds3_init_required = 1,
	.lis3mdl_init_required = 1,
};


// выстывляем ошибки для шины
static void bus_failure(int error)
{
	state.bus_error = error;

	state.bus_init_required = 1;
	state.lis3mdl_init_required = 1;
	state.lsm6ds3_init_required = 1;
}


// выстывляем ошибки для lsm
static void lsm6ds3_failure(int error)
{
	state.lsm6ds3_error = error;
	state.lsm6ds3_init_required = 1;
}


// выстывляем ошибки для lis
static void lis3mdl_failure(int error)
{
	state.lis3mdl_error = error;
	state.lis3mdl_init_required = 1;
}


// переинициализируем шину
static int reinit_bus(void)
{
	if (!state.bus_init_required)
		return 0;

	mems_swrst();

	int error = mems_init_bus();
	if (error)
		bus_failure(error);

	return 0;
}


// переинициализируем lsm
static int reinit_lsm6ds3(void)
{
	if (!state.lsm6ds3_init_required)
		return 0;

	int error = mems_lsm6ds3_init();
	if (error)
		lsm6ds3_failure();

	return 0;
}


// переинициализируем lis
static int reinit_lis3mdl(void)
{
	if (!state.lis3mdl_init_required)
		return 0;

	int error = mems_lis3mdl_init();
	if (error)
		lis3mdl_failure();

	return 0;
}


// обработчик ошибок для lsm
static int lsm6ds3_error_handler(int error)
{
	switch (error)
	{
		case af:
			mems_generate_stop_flag();
			reinit_lsm6ds3();
			return -1;

		default:
			reinit_bus();
			return -1;
	}
	return 0;
}


// обработчик ошибок для lis
static int lis3mdl_error_handler(int error)
{
	switch (error)
	{
		case af:
			mems_generate_stop_flag();
			reinit_lis3mdl();
			return -1;

		default:
			reinit_bus();
			return -1;
	}
	return 0;
}


int sensors_init(void)
{
	reinit_bus();
	if (state.bus_error)
		return state.bus_error;

	reinit_lsm6ds3();
	reinit_lis3mdl();

	if (state.lis3mdl_error)
		return state.lis3mdl_error;
	if (state.lsm6ds3_error)
		return state.lsm6ds3_error;

	return 0;
}


int sensors_lsm6ds3_read(float * accel, float * gyro)
{
	reinit_bus();
	if (state.bus_error)
		return state.bus_error;

	reinit_lsm6ds3();
	if (state.lsm6ds3_error)
		return state.lsm6ds3_error;

	int error = mems_lsm6ds3_get_g_data_rps(gyro);
	if (lsm6ds3_error_handler(error))
		return error;

	error = mems_lsm6ds3_get_xl_data_g(accel);
	if (lsm6ds3_error_handler(error))
		return error;

	return 0;
}


int sensors_lis3mdl_read(float * magn)
{
	reinit_bus();
	if (state.bus_error)
		return state.bus_error;

	reinit_lsm6ds3();
	if (state.lis3mdl_error)
		return state.lis3mdl_error;

	int error = mems_lis3mdl_get_m_data_mG();
	if (lis3mdl_error_handler(error))
		return error;

	return 0;
}
