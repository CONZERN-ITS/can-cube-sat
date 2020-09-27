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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <math.h>

#include <stm32f4xx_hal.h>
#include <diag/Trace.h>

#include "sins_config.h"

#include "drivers/time_svc/time_svc.h"
#include "drivers/mems/mems.h"
#include "drivers/gps/gps.h"
#include "drivers/uplink.h"
#include "drivers/time_svc/timers_world.h"
#include "drivers/temp/analog.h"
#include "backup_sram.h"
#include "drivers/led.h"
#include "errors.h"

#include "mav_packet.h"
#include "watchdog.h"

#include "state.h"

#include "MadgwickAHRS.h"
#include "vector.h"
#include "quaternion.h"
#include "sensors.h"


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

#define    DWT_CYCCNT    *(volatile uint32_t*)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t*)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t*)0xE000EDFC


static void dwt_init()
{
	SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;// разрешаем использовать DWT
	DWT_CONTROL|= DWT_CTRL_CYCCNTENA_Msk; // включаем счётчик
	DWT_CYCCNT = 0;// обнуляем счётчик
}


void system_reset()
{
	led_blink(5, 400);
	HAL_NVIC_SystemReset();
}

static uint32_t last_gps_packet_ts = 0;
static uint32_t last_gps_fix_packet_ts = 0;


static void calibration_accel()
{
	backup_sram_enable();
	backup_sram_erase();

	sensors_init();

	debug_uart_init();

	for(;;)
	{
		//	Arrays
		float accel[3] = {0, 0, 0};
		float gyro[3] = {0, 0, 0};
//		float magn[3] = {0, 0, 0};

//		error_system.lis3mdl_error = sensors_lis3mdl_read(magn);
	//	trace_printf("lis error %d\n", error_system.lis3mdl_init_error);

		error_system.lsm6ds3_error = sensors_lsm6ds3_read(accel, gyro);
	//	trace_printf("lsm error %d\n", error_system.lsm6ds3_init_error);

		if (error_system.lsm6ds3_error != 0)
			continue;

		//	пересчитываем их и записываем в структуры
		stateSINS_isc.tv.tv_sec = HAL_GetTick();

		for (int k = 0; k < 3; k++)
		{

			stateSINS_isc.accel[k] = accel[k];
		}

		mavlink_sins_isc(&stateSINS_isc);
		led_toggle();
	}
}


static void calibration_magn()
{
	backup_sram_enable();
	backup_sram_erase();

	sensors_init();

	debug_uart_init();

	for(;;)
	{
		//	Arrays
//		float accel[3] = {0, 0, 0};
//		float gyro[3] = {0, 0, 0};
		float magn[3] = {0, 0, 0};

		error_system.lis3mdl_error = sensors_lis3mdl_read(magn);

		if (error_system.lis3mdl_error!= 0)
			continue;

		//	пересчитываем их и записываем в структуры
		stateSINS_isc.tv.tv_sec = HAL_GetTick();

		for (int k = 0; k < 3; k++)
		{

			stateSINS_isc.magn[k] = magn[k];
		}

		mavlink_sins_isc(&stateSINS_isc);
		led_toggle();
	}
}


// Функция для слежения за здоровьем GPS и передачи его пакетов в мавлинк
static void on_gps_packet_main(void * arg, const ubx_any_packet_t * packet)
{
	switch (packet->pid)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
	case UBX_PID_NAV_SOL:
		// Отмечаем время получения пакета с фиксом
		if (packet->packet.navsol.gps_fix == UBX_FIX_TYPE__3D)
			last_gps_fix_packet_ts = HAL_GetTick();
#pragma GCC diagnostic pop
		/* no break */

	case UBX_PID_NAV_TIMEGPS:
	case UBX_PID_TIM_TP:
		// Отмечаем время получения вообще какого-либо пакета
		last_gps_packet_ts = HAL_GetTick();
		break;
	}

	// Передаем дальше для обработки
	on_gps_packet(arg, packet);
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

	error_system.lis3mdl_error = sensors_lis3mdl_read(magn);
//	trace_printf("lis error %d\n", error_system.lis3mdl_init_error);

	error_system.lsm6ds3_error = sensors_lsm6ds3_read(accel, gyro);
//	trace_printf("lsm error %d\n", error_system.lsm6ds3_init_error);

	time_svc_world_get_time(&stateSINS_isc.tv);

	//	пересчитываем их и записываем в структуры
	for (int k = 0; k < 3; k++) {
		stateSINS_rsc.accel[k] = accel[k];
		gyro[k] -= state_zero.gyro_staticShift[k];
		stateSINS_rsc.gyro[k] = gyro[k];
		stateSINS_rsc.magn[k] = magn[k];
	}

	if ((error_system.lsm6ds3_error != 0) && (error_system.lis3mdl_error != 0))
		return -22;

	/////////////////////////////////////////////////////
	/////////////	UPDATE QUATERNION  //////////////////
	/////////////////////////////////////////////////////
	float quaternion[4] = {1, 0, 0, 0};


	float dt = ((float)((stateSINS_isc.tv.tv_sec * 1000 + stateSINS_isc.tv.tv_usec / 1000)  - (stateSINS_isc_prev.tv.tv_sec * 1000 + stateSINS_isc_prev.tv.tv_usec / 1000))) / 1000;
//	trace_printf("dt = %f", dt);
	stateSINS_isc_prev.tv.tv_sec = stateSINS_isc.tv.tv_sec;
	stateSINS_isc_prev.tv.tv_usec = stateSINS_isc.tv.tv_usec;


	float beta = 6.0;
	if ((error_system.lsm6ds3_error == 0) && (error_system.lis3mdl_error == 0))
		MadgwickAHRSupdate(quaternion, gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2], magn[0], magn[1], magn[2], dt, beta);
	else if (error_system.lsm6ds3_error == 0)
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

	if (0 == error_system.lsm6ds3_error)
	{
		for (int i = 0; i < 3; i++)
		{
			accel_ISC[i] -= state_zero.accel_staticShift[i];
			stateSINS_isc.accel[i] = accel_ISC[i];
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
			stateSINS_isc.accel[i] = 0;
	}

	//	Copy vectors to global structure
	if (0 == error_system.lis3mdl_error)
	{
		for (int i = 0; i < 3; i++)
		{
			stateSINS_isc.magn[i] = magn[i];
		}
	}
	else
	{
		for(int i = 0; i < 3; i++)
			stateSINS_isc.magn[i] = 0;
	}

	return error;
}


/**
  * @brief	Special function for updating previous values structures by current values
  */
void SINS_updatePrevData(void)
{
	for(int i = 0; i < 3; i++)
		if (isnanf(stateSINS_isc.quaternion[i]))		//проверка на нан
			return;

	__disable_irq();
	memcpy(&stateSINS_isc_prev,			&stateSINS_isc,			sizeof(stateSINS_isc));
//	memcpy(&state_system_prev, 			&state_system,		 	sizeof(state_system));		//FIXME: зачем это делать?
	__enable_irq();
}


int check_SINS_state(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpioc;
	gpioc.Mode = GPIO_MODE_INPUT;
	gpioc.Pin = GPIO_PIN_9;
	gpioc.Pull = GPIO_NOPULL;
	gpioc.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioc);


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

	led_init();

	dwt_init();

	if (CALIBRATION_LSM)
		calibration_accel();

	if (CALIBRATION_LIS)
		calibration_magn();



	if (check_SINS_state() == 1)
	{
		backup_sram_enable();
		backup_sram_erase();

		sensors_init();
		error_mems_read();

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

		error_system.reset_counter = 0;
		backup_sram_write_reset_counter(&error_system.reset_counter);
		backup_sram_write_zero_state(&state_zero);

	}
	else
	{
		iwdg_init(&transfer_uart_iwdg_handle);

		time_svc_steady_init();

		backup_sram_enable_after_reset();
		backup_sram_read_zero_state(&state_zero);

		backup_sram_read_reset_counter(&error_system.reset_counter);
		error_system.reset_counter++;
		backup_sram_write_reset_counter(&error_system.reset_counter);

		int error = time_svc_world_preinit_with_rtc();
		error_system.rtc_error = error;
		if (error != 0)
			time_svc_world_preinit_without_rtc(); 		//не смогли запустить rtc. Запустимся без него
		else
			time_svc_world_init();			//Смогли запустить rtc. Запустим все остальное

		error = 0;
		error = uplink_init();
		error_system.uart_transfer_init_error = error;
		if (error != 0)
			system_reset();				//Если не запустился uart, то мы - кирпич

		error = 0;
		error = gps_init(on_gps_packet_main, NULL);
		gps_configure_begin();
		if (error != 0)
		{
			error_system.gps_uart_init_error = error;
		}
		else
		{
			error_system.gps_config_error = error;
		}

	//	int rc = gps_init(_on_gps_packet, NULL);
	//	trace_printf("configure rc = %d\n", rc);

		error = 0;
		error = analog_init();
		error_system.analog_sensor_init_error = error;
		if (error != 0)
			{
				HAL_Delay(500);
				error_system.analog_sensor_init_error = analog_restart();
			}

		sensors_init();
		error_mems_read();

		time_svc_world_get_time(&stateSINS_isc_prev.tv);

		error_system_check();

		uint32_t prew_time = HAL_GetTick();
		uint32_t time = 0;

		for (; ; )
		{
			for (int u = 0; u < 5; u++)
			{
//				uint32_t x = HAL_GetTick();
				for (int i = 0; i < 30; i++)
				{

					UpdateDataAll();
					SINS_updatePrevData();
				}

//				volatile uint32_t z = HAL_GetTick() - x;

		//		struct timeval tmv;
		//		time_svc_world_timers_get_time(&tmv);
		//		struct tm * tm = gmtime(&tmv.tv_sec);
		//		char buffer[sizeof "2011-10-08T07:07:09Z"] = {0};
		//		strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm);
		//		trace_printf("time is %s\n", buffer);

				gps_poll();

				const int gps_cfg_status = gps_configure_status();
				if (gps_cfg_status != -EWOULDBLOCK) // конфигурация уже закончилась
				{
					error_system.gps_config_error = gps_cfg_status;
					uint32_t now = HAL_GetTick();

					if (gps_cfg_status != 0)
					{
						// закончилась но плохо. Начинаем опять
						error_system.gps_reconfig_counter++;
						gps_configure_begin();
					}
					else if (now - last_gps_fix_packet_ts > ITS_SINS_GPS_MAX_NOFIX_TIME)
					{
						// Если GPS слишком долго не фиксится
						// Тоже отправляем его в реконфигурациюs
						// Перед этим сделаем вид, что фикс у нас был
						// Так как он редко появляется сразу, а если это время не обновить -
						// то эта ветка будет срабатывать постоянно
						error_system.gps_reconfig_counter++;
						last_gps_fix_packet_ts = HAL_GetTick();
						gps_configure_begin();
					}
					else if (now - last_gps_packet_ts > ITS_SINS_GPS_MAX_NOPACKET_TIME)
					{
						// Если слишком давно не приходило интересных нам пакетов
						// Отправляем gps в реконфигурацию
						// Сбросим время,чтобы не крутитсяв этом цикле вечно, если что-то пошло не так
						error_system.gps_reconfig_counter++;
						last_gps_packet_ts = HAL_GetTick();
						gps_configure_begin();
					}
				}

				if ((error_system.lsm6ds3_error != 0) && (error_system.lis3mdl_error != 0))
					continue;
				mavlink_sins_isc(&stateSINS_isc);

			}
			time = HAL_GetTick();
			if (time - prew_time < 1000)
				continue;
			prew_time = time;
			mavlink_timestamp();
			own_temp_packet();

			error_mems_read();
			mavlink_errors_packet();
		}
	}
	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
