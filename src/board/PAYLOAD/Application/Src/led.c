/*
 * led.c
 *
 *  Created on: Jun 14, 2020
 *      Author: snork
 */

#include "led.h"

#include "main.h"

#include <stm32f1xx_hal.h>


void led_init()
{
	// Все настроено уже кубом ...
	// У нас два дублирующих леда. Один будет работать на блипилах
	// (который PC13 или LED_BLUEPILL)
	// второй будет работать на готовой плате.
	// (PB5 или LED_REL_BOARD)

	// FIXME: Так-то нужно бы сделать две конфигурации сборки
	// в которых будет использовать либо тот либо другой
	// но мы пока сделаем чтобы они просто работали одинаково.
}


void led_set(bool state)
{
	// Все наоборот. 0 соответствует включенному леду, потому что он подключен
	// к пину в качестве земли. Пин работает на OpenDrain
	GPIO_PinState pin_state = state ? GPIO_PIN_RESET : GPIO_PIN_SET;

	HAL_GPIO_WritePin(LED_BLUEPILL_GPIO_Port, LED_BLUEPILL_Pin, pin_state);
	HAL_GPIO_WritePin(LED_REL_BOARD_GPIO_Port, LED_REL_BOARD_Pin, pin_state);
}


void led_toggle()
{
	HAL_GPIO_TogglePin(LED_BLUEPILL_GPIO_Port, LED_BLUEPILL_Pin);
	HAL_GPIO_TogglePin(LED_REL_BOARD_GPIO_Port, LED_REL_BOARD_Pin);
}
