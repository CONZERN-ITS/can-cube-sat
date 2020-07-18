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

#include "lwip/ip_addr.h"

#define ITS_I2CTM_DEV_COUNT			2
#define ITS_ARK_ADDRESS 			0x68
#define ITS_PLD_ADDRESS 			0x69

#define ITS_I2CTM_FREQ 				100000
#define ITS_I2CTM_PORT 				I2C_NUM_0

#define ITS_IMI_PORT 				0

#define ITS_UART_PORT 				UART_NUM_2
#define ITS_UART_RX_BUF_SIZE		1024
#define ITS_UART_TX_BUF_SIZE		1024
#define ITS_UART_QUEUE_SIZE			20


#define ITS_UART0_PORT 				UART_NUM_1
#define ITS_UART0_RX_BUF_SIZE		1024
#define ITS_UART0_TX_BUF_SIZE		1024
#define ITS_UART0_QUEUE_SIZE		20

#define ITS_WIFI_SERVER				1
#if !defined(ITS_WIFI_SERVER) || !ITS_WIFI_SERVER
#define ITS_WIFI_CLIENT				1
#else
#define ITS_WIFI_CLIENT				0
#endif


static const ip_addr_t ITS_WIFI_SERVER_ADDRESS = IPADDR4_INIT_BYTES(192, 168, 4, 1);
static const ip_addr_t ITS_WIFI_CLIENT_ADDRESS = IPADDR4_INIT_BYTES(192, 168, 4, 40);
#define ITS_WIFI_PORT 53442

void init_helper(void);


#endif /* MAIN_INIT_HELPER_H_ */
