/*
 * led.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_LED_H_
#define DRIVERS_LED_H_


//! Настройка светодиодов
void led_init(void);
void led_blink(int count, uint32_t timeout);
void led_down(void);
void led_up(void);

void led_toggle(void);

#endif /* DRIVERS_LED_H_ */
