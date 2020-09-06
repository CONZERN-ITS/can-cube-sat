/*
 * sensors.c
 *
 *  Created on: 22 авг. 2020 г.
 *      Author: developer
 */
#include <errno.h>
#include <string.h>

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

	state.lsm6ds3_accel_data_counter = 0;
	state.lsm6ds3_gyro_data_counter = 0;
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
	state.lis3mdl_magn_data_counter = 0;
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


static void lsm6ds3_check_repeat_data(int16_t * data, int16_t * prew_data, int * counter)
{
	if (memcmp(data, prew_data, 3 * sizeof(int16_t)) == 0)
		(*counter)++;
	else
	{
		for(int i = 0; i < 3; i++)
			prew_data[i] = data[i];
		(*counter) = 0;
	}

	if ((*counter) == 5)
	{
		lsm6ds3_failure(44);
		(*counter) = 0;
	}
}


static void lis3mdl_check_repeat_data(int16_t * data, int16_t * prew_data, int * counter)
{
	if (memcmp(data, prew_data, 3 * sizeof(int16_t)) == 0)
		(*counter)++;
	else
	{
		for(int i = 0; i < 3; i++)
			prew_data[i] = data[i];
		(*counter) = 0;
	}

	if ((*counter) == 5)
	{
		lis3mdl_failure(44);
		(*counter) = 0;
	}
}


int sensors_lsm6ds3_read(float * accel, float * gyro)
{
	reinit_bus();
	if (!state.bus_ready)
		return state.bus_error;

	reinit_lsm6ds3();
	if (!state.lsm6ds3_ready)
		return state.lsm6ds3_error;

	int16_t data_gyro[3] = {0};

	int error = mems_lsm6ds3_get_g_data_raw(data_gyro);
	mems_lsm6ds3_get_g_data_rps(data_gyro, gyro);

	lsm6ds3_check_repeat_data(data_gyro, state.gyro_prew, &state.lsm6ds3_gyro_data_counter);

	if (error)
	{
		lsm6ds3_failure(error);
		return error;
	}

	int16_t data_accel[3] = {0};

	error = mems_lsm6ds3_get_xl_data_raw(data_accel);
	mems_lsm6ds3_get_xl_data_g(data_accel, accel);

	lsm6ds3_check_repeat_data(data_accel, state.accel_prew, &state.lsm6ds3_accel_data_counter);

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

	int16_t data_magn[3] = {0};

	int error = mems_lis3mdl_get_m_data_raw(data_magn);
	mems_lis3mdl_get_m_data_mG(data_magn, magn);

	lis3mdl_check_repeat_data(data_magn, state.magn_prew, &state.lis3mdl_magn_data_counter);

	if (error)
	{
		lis3mdl_failure(error);
		return error;
	}

	return 0;
}