/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stm32f4xx_hal.h>
#include <diag/Trace.h>

#include "drivers/time_svc/time_svc.h"
#include "drivers/mems/mems.h"
#include "drivers/gps/gps.h"
#include "drivers/uplink.h"
#include "drivers/time_svc/timers_world.h"
#include "drivers/temp/analog.h"

#include "mav_packet.h"

#include "state.h"

#include "MadgwickAHRS.h"
#include "vector.h"
#include "quaternion.h"
#include "backup_sram.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//Global structures
error_system_t error_system;
state_system_t state_system;
stateSINS_rsc_t stateSINS_rsc;
state_zero_t state_zero;
stateSINS_isc_t stateSINS_isc;
stateSINS_isc_t stateSINS_isc_prev;
//stateSINS_transfer_t stateSINS_transfer;
//stateGPS_t stateGPS;


void system_reset()
{
	led_blink(5, 400);
	HAL_NVIC_SystemReset();
}


void SENSORS_Init(int bus_init, int lsm_init, int lis_init)
{
	int error = 0;
	if (bus_init)
	{
		error = mems_init_bus();
		error_system.i2c_init_error = error;
		if (error != 0)
		{
			HAL_Delay(1000);
			error = mems_init_bus();
			error_system.i2c_init_error = error;
		}
	}

	//	LSM6DS3_init
	if (lsm_init)
	{
		error = 0;
		error = mems_lsm6ds3_init();
		error_system.lsm6ds3_init_error =error;
		if (error != 0)
		{
			HAL_Delay(1000);
			error_system.lsm6ds3_init_error = mems_lsm6ds3_init();
		}
	}

	//	LIS3MDL init
	if (lis_init)
	{
		error = 0;
		error = mems_lis3mdl_init();
		error_system.lis3mdl_init_error = error;
		if (error != 0)
			{
				HAL_Delay(1000);
				error_system.lis3mdl_init_error = mems_lis3mdl_init();
			}
	}
}


/**
  * @brief	Collects data from SINS, stores it and makes quat using S.Madgwick's algo
  * @retval	R/w IMU error
  */
int UpdateDataAll(void)
{
	int error = 0;

	//	Arrays
	float accel[3] = {0, 0, 0};
	float gyro[3] = {0, 0, 0};
	float magn[3] = {0, 0, 0};

	if ((error_system.lsm6ds3_init_error != 0) && (error_system.lis3mdl_init_error != 0) && (error_system.i2c_init_error != 0))
	{
		SENSORS_Init(1, 1, 1);
	}
	else if (error_system.lsm6ds3_init_error != 0)
		SENSORS_Init(0, 1, 0);
	else if (error_system.lis3mdl_init_error != 0)
		SENSORS_Init(0, 0, 1);


	if (error_system.lsm6ds3_init_error == 0)
	{
		error = mems_lsm6ds3_get_xl_data_g(accel);
		error |= mems_lsm6ds3_get_g_data_rps(gyro);
	}

	if (error_system.lis3mdl_init_error == 0)
		error = mems_lis3mdl_get_m_data_mG(magn);

	if ((error_system.lsm6ds3_init_error != 0) && (error_system.lis3mdl_init_error != 0))
	{
		return -1;
	}

	time_svc_world_get_time(&stateSINS_isc.tv);
	//	пересчитываем их и записываем в структуры
	for (int k = 0; k < 3; k++) {
		stateSINS_rsc.accel[k] = accel[k];
		gyro[k] -= state_zero.gyro_staticShift[k];
		stateSINS_rsc.gyro[k] = gyro[k];
		stateSINS_rsc.magn[k] = magn[k];
	}

	/////////////////////////////////////////////////////
	/////////////	UPDATE QUATERNION  //////////////////
	/////////////////////////////////////////////////////
	float quaternion[4] = {0, 0, 0, 0};


	float dt = ((float)((stateSINS_isc.tv.tv_sec * 1000 + stateSINS_isc.tv.tv_usec / 1000)  - (stateSINS_isc_prev.tv.tv_sec * 1000 + stateSINS_isc_prev.tv.tv_usec / 1000))) / 1000;
//	trace_printf("dt = %f", dt);
	stateSINS_isc_prev.tv.tv_sec = stateSINS_isc.tv.tv_sec;
	stateSINS_isc_prev.tv.tv_usec = stateSINS_isc.tv.tv_usec;


	float beta = 0.33;
	if ((error_system.lsm6ds3_init_error == 0) && (error_system.lis3mdl_init_error == 0))
		MadgwickAHRSupdate(quaternion, gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2], magn[0], magn[1], magn[2], dt, beta);
	else if (error_system.lsm6ds3_init_error == 0)
		MadgwickAHRSupdateIMU(quaternion, gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2], dt, beta);

	//	копируем кватернион в глобальную структуру
	stateSINS_isc.quaternion[0] = quaternion[0];
	stateSINS_isc.quaternion[1] = quaternion[1];
	stateSINS_isc.quaternion[2] = quaternion[2];
	stateSINS_isc.quaternion[3] = quaternion[3];

	/////////////////////////////////////////////////////
	///////////  ROTATE VECTORS TO ISC  /////////////////
	/////////////////////////////////////////////////////

	float accel_ISC[3] = {0, 0, 0};
	vect_rotate(accel, quaternion, accel_ISC);

	//	Copy vectors to global structure
	for (int i = 0; i < 3; i++)
	{
		accel_ISC[i] -= state_zero.accel_staticShift[i];
		stateSINS_isc.accel[i] = accel_ISC[i];
		stateSINS_isc.magn[i] = magn[i];
	}

	return error;
}


/**
  * @brief	Special function for updating previous values structures by current values
  */
void SINS_updatePrevData(void)
{
	__disable_irq();
	memcpy(&stateSINS_isc_prev,			&stateSINS_isc,			sizeof(stateSINS_isc));
//	memcpy(&state_system_prev, 			&state_system,		 	sizeof(state_system));		//FIXME: зачем это делать?
	__enable_irq();
}


int check_SINS_state(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)) //пин с джампером
		return 1;
	else
		return 0;
}


int main(int argc, char* argv[])
{

	//	Global structures init
	memset(&stateSINS_isc, 			0x00, sizeof(stateSINS_isc));
	memset(&stateSINS_isc_prev, 	0x00, sizeof(stateSINS_isc_prev));
	memset(&stateSINS_rsc, 			0x00, sizeof(stateSINS_rsc));
	memset(&state_system,			0x00, sizeof(state_system));
	memset(&state_zero,				0x00, sizeof(state_zero));
	memset(&error_system, 			0x00, sizeof(error_system));

	if (check_SINS_state())
	{
		backup_sram_enable();
		backup_sram_erase();

		SENSORS_Init(1, 1, 1);
		HAL_Delay(1000);
		int error;
		for (int i = 0; i < 2; i++)
		{
			error = mems_get_gyro_staticShift(state_zero.gyro_staticShift);
			error += mems_get_accel_staticShift(state_zero.accel_staticShift);
			if (error != 0)
			{
				system_reset();
			}
		}

		backup_sram_write(&state_zero);

	}
	else
	{
		time_svc_steady_init();

		int error = time_svc_world_preinit_with_rtc();
		error_system.rtc_error = error;
		if (error != 0)
			time_svc_world_preinit_without_rtc(); 		//не смогли запустить rtc. Запустимся без него
		else
			time_svc_world_init();			//Смогли запуслить rtc. Запустим все остальное

		error = 0;
		error = uplink_init();
		error_system.uart_transfer_init_error = error;
		if (error != 0)
			system_reset();				//Если не запустился uart, то мы - кирпич

		error = 0;
		error = gps_init(on_gps_packet, NULL);
		if (error != 0)
		{
			error_system.gps_uart_init_error = error;
		}
		else
		{
			error = gps_configure();
			error_system.gps_config_error = error;
			if (error != 0)
				for (int i = 0; i < 5; i++)
				{
					HAL_Delay(1000);
					error = gps_configure();
					if (error == 0)
						break;
				}
			error_system.gps_config_error = error;
		}

	//	int rc = gps_init(_on_gps_packet, NULL);
	//	trace_printf("configure rc = %d\n", rc);

		error = 0;
		error = analog_init();
		error_system.analog_sensor_init_error;
		if (error != 0)
			{
				HAL_Delay(500);
				error_system.analog_sensor_init_error = analog_restart();
			}

		SENSORS_Init(1, 1, 1);

		backup_sram_enable_after_reset();
		backup_sram_read(&state_zero);

		time_svc_world_get_time(&stateSINS_isc_prev.tv);
		for (; ; )
		{
			for (int u = 0; u < 5; u++)
			{
				for (int i = 0; i < 160; i++)
				{
					UpdateDataAll();
					SINS_updatePrevData();
					gps_poll();
				}

		//		struct timeval tmv;
		//		time_svc_world_timers_get_time(&tmv);
		//		struct tm * tm = gmtime(&tmv.tv_sec);
		//		char buffer[sizeof "2011-10-08T07:07:09Z"] = {0};
		//		strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm);
		//		trace_printf("time is %s\n", buffer);


				mavlink_sins_isc(&stateSINS_isc);
				gps_poll();
			}
			mavlink_timestamp();
			own_temp_packet();
		}
	}
	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
