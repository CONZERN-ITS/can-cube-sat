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
// Скидывать сообщения в текстовом виде в консольку для отладки
#ifdef DEBUG
#define PROCESS_TO_PRINTF
#endif
// Скидывать сообщения в its_link
#define PROCESS_TO_ITSLINK


#include <mavlink/mavlink_types.h>

//! Определяем идентификаторы системы
extern mavlink_system_t mavlink_system;

//! Отправка mavlink пакета в i2c-link
void mav_main_send_to_its_link(mavlink_channel_t channel, const uint8_t * buffer, uint16_t buffer_size);
//! Получение mavlink пакета и i2c-link
//! Возвращает 0 если пакет есть и ошибку если ошибка (включая EAGAIN)
int mav_main_get_packet(mavlink_message_t * msg);

//! Определяем функцию для отправки телеметрии
#define MAVLINK_SEND_UART_BYTES mav_main_send_to_its_link

// Наконец то подключаем сам мавлинк
#include <mavlink/its/mavlink.h>

void mav_main_process_bme_message(const mavlink_pld_bme280_data_t * msg);
void mav_main_process_me2o2_message(mavlink_pld_me2o2_data_t * msg);
void mav_main_process_mics_message(mavlink_pld_mics_6814_data_t * msg);
void mav_main_process_owntemp_message(mavlink_own_temp_t * msg);
void mav_main_process_own_stats(mavlink_pld_stats_t * msg);
void mav_main_process_i2c_link_stats(mavlink_i2c_link_stats_t * msg);

#endif /* INC_MAVLINK_MAIN_H_ */
