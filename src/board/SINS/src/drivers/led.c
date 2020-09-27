/*
 * led.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */


#include <stm32f4xx_hal.h>



#define LED_PIN GPIO_PIN_12
#define LED_PORT GPIOC
#define LED_CLOCK __HAL_RCC_GPIOC_CLK_ENABLE()


void led_up(void)
{
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, SET);
}


void led_down(void)
{
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, RESET);
}


void led_toggle(void)
{
	HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}


void led_init(void)
{
	LED_CLOCK;

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
