/*
 * watchdog.h
 *
 *  Created on: 29 авг. 2020 г.
 *      Author: developer
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

extern IWDG_HandleTypeDef transfer_uart_iwdg_handle;
extern IWDG_HandleTypeDef gps_iwdg_handle;

int iwdg_init();

void iwdg_reload();


#endif /* WATCHDOG_H_ */
