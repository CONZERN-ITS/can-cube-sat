/*
 * stm32f4xx_hal_systick.c
 *
 *  Created on: 9 апр. 2020 г.
 *      Author: snork
 */

#include <stm32f4xx_hal.h>


void SysTick_Handler (void)
{
	HAL_IncTick();
}


