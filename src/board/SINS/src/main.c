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

#include <stdio.h>
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

#include "mav_packet.h"

#include "state.h"

#include "MadgwickAHRS.h"
#include "vector.h"
#include "quaternion.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//Global structures
state_system_t state_system;
stateSINS_rsc_t stateSINS_rsc;
state_zero_t state_zero;
stateSINS_isc_t stateSINS_isc;
stateSINS_isc_t stateSINS_isc_prev;
//stateSINS_transfer_t stateSINS_transfer;
//stateGPS_t stateGPS;


// GENERATE PPS	\\//
TIM_HandleTypeDef htim1;

static void MX_TIM1_Init(void)
{
  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 33599;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 4999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
//  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
//    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
//    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
//    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
//    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
//    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
//    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

// GENERATE PPS	\\//


void SENSORS_Init(void)
{
	int error = 0;
	error = mems_init_bus();
//	trace_printf("mems bus init error: %d\n", error);

	//	LSM6DS3_init
	error = mems_lsm6ds3_init();
//	trace_printf("lsm6ds3 init error: %d\n", error);
	state_system.lsm6ds3_state = error;

	//	LIS3MDL init
	error = mems_lis3mdl_init();
//	trace_printf("lis3mdl init error: %d\n", error);
	state_system.lis3mdl_state = error;

//	state_system.GPS_state = error;
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

	error = mems_lsm6ds3_get_xl_data_g(accel);
	error |= mems_lsm6ds3_get_g_data_rps(gyro);
	/*if (error)
	{
		state_system.lsm6ds3_state = error;				//FIXME: подумать, надо ли возвращать
		goto end;
	}
*/
	error = mems_lis3mdl_get_m_data_mG(magn);
/*	if (error)
	{
		state_system.lis3mdl_state = error;				//FIXME: подумать, надо ли возвращать
		goto end;
	}
*/
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
	MadgwickAHRSupdate(quaternion, gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2], magn[0], magn[1], magn[2], dt, beta);

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

end:
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

void start_handmade_pps(void)
{
	MX_TIM1_Init();
	__HAL_TIM_MOE_ENABLE(&htim1);
	__HAL_TIM_SET_COUNTER(&htim1, __HAL_TIM_GET_AUTORELOAD(&htim1));
	TIM_CCxChannelCmd(htim1.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

	HAL_Delay(10);

	__HAL_TIM_ENABLE(&htim1);
}

//FIXME: реализовать таймер для отправки пакетов с мк по uart


int main(int argc, char* argv[])
{
//	__disable_irq();
	//	Global structures init
	memset(&stateSINS_isc, 			0x00, sizeof(stateSINS_isc));
	memset(&stateSINS_isc_prev, 	0x00, sizeof(stateSINS_isc_prev));
	memset(&stateSINS_rsc, 			0x00, sizeof(stateSINS_rsc));
	memset(&state_system,			0x00, sizeof(state_system));
	memset(&state_zero,				0x00, sizeof(state_zero));

	// FIXME: сделать таймер для маджвика на микросекунды, возможно привязанный к HAL_GetTick()

	assert(0 == time_svc_steady_init());
	assert(0 == time_svc_world_init());
	assert(0 == gps_init(_on_gps_packet, NULL));

	uplink_init();

	int rc = gps_init(_on_gps_packet, NULL);
	rc = gps_configure();
	trace_printf("configure rc = %d\n", rc);

	SENSORS_Init();
	for (int i = 0; i < 2; i++)
	{
		mems_get_gyro_staticShift(state_zero.gyro_staticShift);
		mems_get_accel_staticShift(state_zero.accel_staticShift);
	}

	time_svc_world_get_time(&stateSINS_isc_prev.tv);
	for (; ; )
	{
		for (int i = 0; i < 1; i++)
		{
			UpdateDataAll();
			SINS_updatePrevData();
			gps_poll();
		}


		_mavlink_sins_isc(&stateSINS_isc);
		_mavlink_timestamp();
		gps_poll();
	}

	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
