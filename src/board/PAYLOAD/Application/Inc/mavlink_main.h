/*
 * mavlink_main.h
 *
 *  Created on: Jun 14, 2020
 *      Author: snork
 */

#ifndef INC_MAVLINK_MAIN_H_
#define INC_MAVLINK_MAIN_H_

#include <stddef.h>

//! Уменьшаем количество буферов мавлинка до одного
#define MAVLINK_COMM_NUM_BUFFERS 1

//! Включаем удобные мавлинковые функции
#define MAVLINK_USE_CONVENIENCE_FUNCTIONS

#include <mavlink/mavlink_types.h>

//! Определяем идентификаторы системы
extern mavlink_system_t mavlink_system;

//! Отправка mavlink пакета в i2c-link
void mavlink_main_send_to_its_link(mavlink_channel_t channel, const uint8_t * buffer, uint16_t buffer_size);
//! Получение mavlink пакета и i2c-link
//! Возвращает 0 если пакет есть и ошибку если ошибка (включая EAGAIN)
int mavlink_main_get_packet(mavlink_message_t * msg);

//! Определяем функцию для отправки телеметрии
#define MAVLINK_SEND_UART_BYTES mavlink_main_send_to_its_link

// Наконец то подключаем сам мавлинк
#include <mavlink/its/mavlink.h>

#endif /* INC_MAVLINK_MAIN_H_ */
