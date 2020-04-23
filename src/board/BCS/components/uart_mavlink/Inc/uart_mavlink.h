/*
 * uart_mavlink.h
 *
 *  Created on: 23 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef COMPONENTS_UART_MAVLINK_INC_UART_MAVLINK_H_
#define COMPONENTS_UART_MAVLINK_INC_UART_MAVLINK_H_

#include "driver/uart.h"
#include "freertos/queue.h"

void uart_mavlink_install(uart_port_t uart_num, QueueHandle_t uart_queue);


#endif /* COMPONENTS_UART_MAVLINK_INC_UART_MAVLINK_H_ */
