/*
 * led.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */


#include <stm32f4xx_hal.h>


void led_init()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, RESET);

	GPIO_InitTypeDef gpioc;
	gpioc.Mode = GPIO_MODE_OUTPUT_PP;
	gpioc.Pin = GPIO_PIN_12;
	gpioc.Pull = GPIO_NOPULL;
	gpioc.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &gpioc);  //FIXME: on GPIOC and port 12


}
