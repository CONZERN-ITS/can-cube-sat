/*
 * watchdog.c
 *
 *  Created on: 29 авг. 2020 г.
 *      Author: developer
 */

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_iwdg.h"


IWDG_HandleTypeDef transfer_uart_iwdg_handle = {
	.Instance = IWDG,
	.Init = {
		.Prescaler = IWDG_PRESCALER_128,
		.Reload = 0xfff,
	},
};


int iwdg_init(IWDG_HandleTypeDef * handle)
{

	int error = HAL_IWDG_Init(handle);
	return error;
}

void iwdg_reload(IWDG_HandleTypeDef * handle)
{
	HAL_IWDG_Refresh(handle);
}

