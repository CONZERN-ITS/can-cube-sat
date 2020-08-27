/*
 * sensors.c
 *
 *  Created on: 22 авг. 2020 г.
 *      Author: developer
 */
#include <errno.h>

#include "sensors.h"
#include "drivers/mems/mems.h"
#include "errors.h"


i2c_error_codes i2c_errors;

mems_state_t state = { 0 };


// выстывляем ошибки для шины
static void bus_failure(int error)
{
	state.bus_error = error;
	state.bus_error_counter++;

	state.bus_ready = 0;
	state.lis3mdl_ready = 0;
	state.lsm6ds3_ready = 0;
}


// выстывляем ошибки для lsm
static void lsm6ds3_failure(int error)
{
	state.lsm6ds3_error_counter++;
	switch (error)
	{
		case af :
			state.lsm6ds3_error = error;
			state.lsm6ds3_ready = 0;
			break;

		case (-ENODEV):
			state.lsm6ds3_error = error;
			state.lsm6ds3_ready = 0;
			break;

		default:
			bus_failure(error);
			state.lsm6ds3_error = error;
			break;
	};
}


// выстывляем ошибки для lis
static void lis3mdl_failure(int error)
{
	state.lis3mdl_error_counter++;
	switch (error)
	{
		case af:
			state.lis3mdl_error = error;
			state.lis3mdl_ready = 0;
			break;

		case (-ENODEV):
			state.lis3mdl_error = error;
			state.lis3mdl_ready = 0;
			break;

		default:
			bus_failure(error);
			state.lis3mdl_error = error;
			break;
	};
}


// переинициализируем шину
static int reinit_bus(void)
{
	if (state.bus_ready)
		return 0;

	int error = mems_init_bus();
	if (error)
	{
		bus_failure(error);
		return error;
	}

	state.bus_ready = 1;
	return 0;
}


// переинициализируем lsm
static int reinit_lsm6ds3(void)
{
	if (state.lsm6ds3_ready)
		return 0;

	int error = mems_lsm6ds3_init();
	if (error)
	{
		lsm6ds3_failure(error);
		return error;
	}

	state.lsm6ds3_ready = 1;
	return 0;
}


// переинициализируем lis
static int reinit_lis3mdl(void)
{
	if (state.lis3mdl_ready)
		return 0;

	int error = mems_lis3mdl_init();
	if (error)
	{
		lis3mdl_failure(error);
		return error;
	}

	state.lis3mdl_ready = 1;
	return 0;
}


int sensors_init(void)
{
	reinit_bus();
	if (!state.bus_ready)
		return state.bus_error;

	reinit_lsm6ds3();
	reinit_lis3mdl();

	if (!state.lis3mdl_ready)
		return state.lis3mdl_error;
	if (!state.lsm6ds3_ready)
		return state.lsm6ds3_error;

	return 0;
}


int sensors_lsm6ds3_read(float * accel, float * gyro)
{
	reinit_bus();
	if (!state.bus_ready)
		return state.bus_error;

	reinit_lsm6ds3();
	if (!state.lsm6ds3_ready)
		return state.lsm6ds3_error;

	int error = mems_lsm6ds3_get_g_data_rps(gyro);
	if (error)
	{
		lsm6ds3_failure(error);
		return error;
	}

	error = mems_lsm6ds3_get_xl_data_g(accel);
	if (error)
	{
		lsm6ds3_failure(error);
		return error;
	}

	return 0;
}


int sensors_lis3mdl_read(float * magn)
{
	reinit_bus();
	if (!state.bus_ready)
		return state.bus_error;

	reinit_lis3mdl();
	if (!state.lis3mdl_ready)
		return state.lis3mdl_error;

	int error = mems_lis3mdl_get_m_data_mG(magn);
	if (error)
	{
		lis3mdl_failure(error);
		return error;
	}

	return 0;
}
