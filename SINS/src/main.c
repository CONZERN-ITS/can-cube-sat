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
#include "diag/Trace.h"

#include "Timer.h"
#include "BlinkLed.h"
#include "state.h"

#include "drivers/lis3mdl.h"
#include "drivers/lsm6ds3.h"


SPI_HandleTypeDef spi;

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 led blink sample (trace via DEBUG).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- Timing definitions -------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//Global structures
state_system_t state_system;
stateSINS_rsc_t stateSINS_rsc;
state_zero_t state_zero;
state_system_t state_system_prev;
stateSINS_isc_t stateSINS_isc;
stateSINS_isc_t stateSINS_isc_prev;

static uint8_t	get_gyro_staticShift(float* gyro_staticShift);
static uint8_t	get_accel_staticShift(float* gyro_staticShift, float* accel_staticShift);
void get_staticShifts();

void IMU_Init();
int IMU_updateDataAll();
void _IMUtask_updateData();


/**
  * @brief	Static function used to get gyro static shift
  * @param	gyro_staticShift	Array used to store that shift
  * @retval	Device's wire error
  */
static uint8_t get_gyro_staticShift(float* gyro_staticShift)
{
	uint8_t error = 0;
	uint16_t zero_orientCnt = 2000;

	float gyro[3] = {0, 0, 0};

	//	Get static gyro shift
	for (int i = 0; i < zero_orientCnt; i++)
	{
		//	Collect data
		PROCESS_ERROR(lsm6ds3_get_g_data_rps(gyro));

		for (int m = 0; m < 3; m++)
			gyro_staticShift[m] += gyro[m];
	}
	for (int m = 0; m < 3; m++) {
		gyro_staticShift[m] /= zero_orientCnt;
	}
end:
	return error;
}


/**
  * @brief	Static function used to get accel static shift
  * @param	gyro_staticShift	Array used to get gyro shift from
  * @param	accel_staticShift	Array used to store accel shift
  * @retval	Device's wire error
  */
static uint8_t get_accel_staticShift(float* gyro_staticShift, float* accel_staticShift)
{
	uint8_t error = 0;
	uint16_t zero_orientCnt = 300;

	for (int i = 0; i < zero_orientCnt; i++)
	{
		float accel[3] = {0, 0, 0};
		float gyro[3] = {0, 0, 0};

		//	Collect data
		PROCESS_ERROR(lsm6ds3_get_xl_data_g(accel));
		PROCESS_ERROR(lsm6ds3_get_g_data_rps(gyro));

		for (int k = 0; k < 3; k++) {
			gyro[k] -= gyro_staticShift[k];
		}

		//	Set accel static shift vector as (0,0,g)
		accel_staticShift[0] = 0;
		accel_staticShift[1] = 0;
		accel_staticShift[2] += sqrt(accel[0]*accel[0] + accel[1]*accel[1] + accel[2]*accel[2]);
	}

	//	Divide shift by counter
	for (int m = 0; m < 3; m++)
		accel_staticShift[m] /= zero_orientCnt;

end:
	return error;
}


/**
  * @brief	Initializes I2C for IMU and IMU too
  */
void SENSORS_Init(void)
{
	int error = 0;
	//	LSM6DS3 init
	error = lsm6ds3_init();
	trace_printf("lsm6ds3 init error: %d\n", error);
	state_system.lsm6ds3_state = error;

	//	LIS3MDL init
	error = lis3mdl_init();
	trace_printf("lis3mdl init error: %d\n", error);
	state_system.lis3mdl_state= error;
}


/**
  * @brief	Collects data from IMU, stores it and makes quat using S.Madgwick's algo
  * @retval	R/w IMU error
  */
int UpdateDataAll(void)
{
	int error = 0;

	//	Arrays
	float accel[3] = {0, 0, 0};
	float gyro[3] = {0, 0, 0};
	float magn[3] = {0, 0, 0};

	error = lsm6ds3_get_xl_data_g(accel);
	error |= lsm6ds3_get_g_data_rps(gyro);
	if (error)
	{
		state_system.lsm6ds3_state = error;
		goto end;
	}

	error = lis3mdl_get_m_data_mG(magn);
	if (error)
	{
		state_system.lis3mdl_state = error;
		goto end;
	}

	float _time = (float)HAL_GetTick() / 1000;
	state_system.time = _time;
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

//	taskENTER_CRITICAL();
		float dt = _time - state_system_prev.time;
//	taskEXIT_CRITICAL();

		float beta = 0.041;
		MadgwickAHRSupdate(quaternion, gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2], magn[0], magn[1], magn[2], dt, beta);

		//	копируем кватернион в глобальную структуру
//	taskENTER_CRITICAL();
		stateSINS_isc.quaternion[0] = quaternion[0];
		stateSINS_isc.quaternion[1] = quaternion[1];
		stateSINS_isc.quaternion[2] = quaternion[2];
		stateSINS_isc.quaternion[3] = quaternion[3];
//	taskEXIT_CRITICAL();


	/////////////////////////////////////////////////////
	///////////  ROTATE VECTORS TO ISC  /////////////////
	/////////////////////////////////////////////////////

		float accel_ISC[3] = {0, 0, 0};
		vect_rotate(accel, quaternion, accel_ISC);

		//	Copy vectors to global structure
		for (int i = 0; i < 3; i++)
			accel_ISC[i] -= state_zero.accel_staticShift[i];

		stateSINS_isc.accel[0] = accel_ISC[0];
		stateSINS_isc.accel[1] = accel_ISC[1];
		stateSINS_isc.accel[2] = accel_ISC[2];
		stateSINS_isc.magn[0] = magn[0];
		stateSINS_isc.magn[1] = magn[1];
		stateSINS_isc.magn[2] = magn[2];

end:
//	if (error)
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
//	else
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
// TODO: wtf?
	return error;
}


/**
  * @brief	Special function for updating previous values structures by current values
  */
void SINS_updatePrevData(void)
{
	memcpy(&stateSINS_isc_prev,			&stateSINS_isc,			sizeof(stateSINS_isc));
	memcpy(&state_system_prev, 			&state_system,		 	sizeof(state_system));
}



int32_t bus_init(void* handle)
{
	int error = 0;
	if (handle == &spi)
	{
		//	SPI init
		spi.Instance = SPI1;
		spi.Init.Mode = SPI_MODE_MASTER;
		spi.Init.Direction = SPI_DIRECTION_2LINES;
		spi.Init.DataSize = SPI_DATASIZE_8BIT;
		spi.Init.CLKPolarity = SPI_POLARITY_LOW;
		spi.Init.CLKPhase = SPI_PHASE_1EDGE;
		spi.Init.NSS = SPI_NSS_SOFT;
		spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
		spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
		spi.Init.TIMode = SPI_TIMODE_DISABLED;
		spi.Init.CRCCalculation = SPI_CRCCALCULATION_ENABLED;
		spi.Init.CRCPolynomial = 7;

		error |= HAL_SPI_Init(&spi);
		HAL_Delay(200);
		trace_printf("spi init error: %d\n", error);
	}
	else
	{
		trace_printf("invalid spi handle\n");
		error = -19;
	}

	return error;
}


int main(int argc, char* argv[])
{
	//	Global structures init
//	memset(&stateIMU_rsc, 			0x00, sizeof(stateIMU_rsc));
//	memset(&stateIMU_isc, 			0x00, sizeof(stateIMU_isc));
//	memset(&state_system, 			0x00, sizeof(state_system));
//
//	memset(&stateIMU_isc_prev, 		0x00, sizeof(stateIMU_isc_prev));
//	memset(&state_system_prev, 		0x00, sizeof(state_system_prev));
//
//	state_system.MPU_state = 111;
//	state_system.NRF_state = 111;
//
//
	init_led();
	bus_init(&spi);
//
//	if (DBGU)
//		_init_usart_dbg();
//
//	//	Peripheral initialization
//	if (IMU)
//	{
//		if (IMU_CALIBRATION)
//			trace_printf("IMU calibration enable\n");
//
//		IMU_Init();
//		get_staticShifts();
//	}
//
//	if (RF)
//		TM_Init();


	for (; ; )
	{



		HAL_Delay(10);
	}

	return 0;
}


void init_led(void){
	GPIO_InitTypeDef gpioc;
	gpioc.Mode = GPIO_MODE_OUTPUT_PP;
	gpioc.Pin = GPIO_PIN_12;
	gpioc.Pull = GPIO_NOPULL;
	gpioc.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &gpioc);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, SET);
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
