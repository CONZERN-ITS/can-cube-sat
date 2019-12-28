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
