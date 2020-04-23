/*
 * pin_init.h
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef MAIN_INIT_HELPER_H_
#define MAIN_INIT_HELPER_H_

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define ITS_IMI_DEVICE_COUNT 		1
#define ITS_ARK_1_ADDRESS 			0x68

#define ITS_I2CTM_FREQ 				50000
#define ITS_I2CTM_PORT 				I2C_NUM_0

#define ITS_UART_PORT 				UART_NUM_1
#define ITS_UART_RX_BUF_SIZE		1024
#define ITS_UART_TX_BUF_SIZE		1024
#define ITS_UART_QUEUE_SIZE			20


#define ITS_UART0_PORT 				UART_NUM_1
#define ITS_UART0_RX_BUF_SIZE		1024
#define ITS_UART0_TX_BUF_SIZE		1024
#define ITS_UART0_QUEUE_SIZE		20

void init_helper(void);


#endif /* MAIN_INIT_HELPER_H_ */