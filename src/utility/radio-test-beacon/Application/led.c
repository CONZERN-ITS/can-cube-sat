#include "led.h"

#include "main.h"


void led_init(void)
{
	// Все настраивается в кубмх
}


void led_toggle(void)
{
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}


void led_set(bool state)
{
	// Обратная полярность, так как пин работает как сток
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, state ? GPIO_PIN_RESET: GPIO_PIN_SET);
}
