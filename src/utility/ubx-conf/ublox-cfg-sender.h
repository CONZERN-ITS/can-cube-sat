#ifndef UBLOX_CFG_SENDER_H_
#define UBLOX_CFG_SENDER_H_

#include <stdint.h>
#include <stddef.h>

//! Контекст настройщика GPS модуля
typedef struct ubx_cfg_sender_ctx_t
{
	//! Указатель на "массив" настроечных сообщений
	const uint8_t ** cfg_msg_defs;

	//! Функция для записи данные в uart
	/*! user_arg - как записан в контексте
		data - байты, которые нужно записать
		data_size - количество байтов, которые нужно записать

		функция должна возвращать >= 0 при успехе и <0 при ошибке
		или отрицательный код в случае ошибки */
	int (*uart_write)(void * /*user_arg*/, const uint8_t * data, int data_size);

	//! Пользовательский аргумент для функции uart_write
	void * user_arg;
} ubx_cfg_sender_ctx_t;


int ubx_cfg_sender_send_cfg(ubx_cfg_sender_ctx_t * ctx);

#endif /* UBLOX_CFG_SENDER_H_ */
