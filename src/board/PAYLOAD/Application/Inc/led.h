/*
 * led.h
 *
 *  Created on: Jun 14, 2020
 *      Author: snork
 */

#ifndef SRC_LED_H_
#define SRC_LED_H_

#include <stdbool.h>

//! Управление лампочками на плате
//! Подробнее в коде самой функции
void led_init(void);

//! Явное управление ледом.
//*! true - включен, false - выключен */
void led_set(bool state);

//! Переключить пин в другое состояние (выкл, если сейчас вкл и наоборот)
void led_toggle(void);


#endif /* SRC_LED_H_ */
