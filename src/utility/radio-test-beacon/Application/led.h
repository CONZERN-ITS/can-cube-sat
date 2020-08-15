/*
 * led.h
 *
 *  Created on: Aug 15, 2020
 *      Author: snork
 */

#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

void led_init(void);

void led_toggle(void);

void led_set(bool state);


#endif /* LED_H_ */
