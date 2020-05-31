/*
 * config.h
 *
 *  Created on: May 30, 2020
 *      Author: Danich
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_


//! Количество попыток на отправку одного и тот же конфигурационного сообщения
//! В UBX приёмник
#define ITS_SINS_GPS_CONFIGURE_ATTEMPTS (5)
//! Таймаут на ожидание ACK/NACK пакета при конфигурации GPS (в мс)
#define ITS_SINS_GPS_CONFIGURE_TIMEOUT (3000)

//! Размер для циклобуфера уарта, по которому приходят входящие GPS сообщения
#define ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE (1500)

//! Размер линейного буфера для накопления UBX сообщений
/*! Должен быть не меньше чем самое большое обрабатываемое UBX сообщение */
#define ITS_SINS_GPS_UBX_SPARSER_BUFFER_SIZE (100)

//! Максимальное количество байт обрабатываемое за один вызов gps_poll
#define ITS_SINS_GPS_MAX_POLL_SIZE (ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE)


#endif /* INC_CONFIG_H_ */
