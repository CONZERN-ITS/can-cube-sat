/*
 * led.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_LED_H_
#define DRIVERS_LED_H_


//! Настройка светодиодов
extern void led_init(void);
extern void led_blink(int count, uint32_t timeout);
extern void led_down();
extern void led_up();


#endif /* DRIVERS_LED_H_ */
