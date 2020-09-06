/*
 * led.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */


#include <stm32f4xx_hal.h>

#define LED_PIN GPIO_PIN_12
#define LED_PORT GPIOC


void led_up()
{
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, SET);
}


void led_down()
{
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, RESET);
}


void led_init()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

//	led_down();

	GPIO_InitTypeDef gpioc;
	gpioc.Mode = GPIO_MODE_OUTPUT_PP;
	gpioc.Pin = LED_PIN;
	gpioc.Pull = GPIO_NOPULL;
	gpioc.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(LED_PORT, &gpioc);  //FIXME: on GPIOC and port 12

	led_up();
}

void led_blink(int count, uint32_t timeout)
{
	for (int i = 0; i < count; i++)
	{
		led_down();
		HAL_Delay(timeout / 2);
		led_up();
		HAL_Delay(timeout / 2);
	}
}
