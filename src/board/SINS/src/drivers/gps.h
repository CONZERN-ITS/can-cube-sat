/*
 * gps.h
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#ifndef DRIVERS_GPS_H_
#define DRIVERS_GPS_H_

#include "ubx_stream_parser.h"


//! Колбек для передачи gps сообщений
/*! Почему не ubx_sparser_packet_callback_t? Потому что инкапсуляция! */
typedef ubx_sparser_packet_callback_t gps_packet_callback_t;

//! Колбек, который вызывается по приходу PPS метки от ГНСС приёмника
typedef void (*gps_pps_callback_t)(void * user_arg);


//! Инициализация драйвера GPS
/*! Настройка нужной перефирии и потокового парсера GPS */
void gps_init(
		gps_packet_callback_t packet_callback, void * packet_callback_arg
);

//! Обработка накопленных в прерывании байт и поиск в них сообщений.
/*! Передает сообщения через указанный ранее колбек. Возвращает количество байт, которое было опрошено */
int gps_poll(void);


#endif /* DRIVERS_GPS_H_ */
