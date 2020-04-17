/*
 * gps.h
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#ifndef INC_UBX_STREAM_PARSER_H_
#define INC_UBX_STREAM_PARSER_H_


#include <stdint.h>
#include <stddef.h>

#include "ubx_packet_parser.h"


//! Сигнатура колбека, которая будет вызвана для успешно разобранного пакета
typedef void (*ubx_sparser_packet_callback_t)(
		//! Пользовательский аргумент
		void * /*user_arg*/,
		//! Разобранные данные пакета
		const ubx_any_packet_t * /*packet*/
);


typedef struct ubx_sparser_ctx_t
{
	//! Состояние парсера
	int state;
	//! Буфер для поиска синхрослова
	uint16_t sw_buffer;
	//! Буфер для накопления данных пакета
	uint8_t * pbuffer;
	//! Размер буфера для накопления данных пакетаs
	uint16_t pbuffer_capacity;
	//! Количество накопленных байт в packet_buffer
	size_t pbuffer_size;
	//! Ожидаемая длина пакета
	size_t expected_packet_size;

	ubx_sparser_packet_callback_t packet_callback;
	void * packet_callback_userarg;
} ubx_sparser_ctx_t;


//! Сброс контекста парсера в исходное состояние
void ubx_sparser_reset(ubx_sparser_ctx_t * ctx);


//! Установка колбека для разбобранных пакетов
void ubx_sparser_set_packet_callback(
		ubx_sparser_ctx_t * ctx,
		ubx_sparser_packet_callback_t callback,
		void * userarg
);

//! Установка буфера для накопления данных пакета
void ubx_sparser_set_pbuffer(ubx_sparser_ctx_t * ctx, uint8_t * pbuffer, uint16_t pbuffer_capacity);

//! Получение очередного байта из потока
void ubx_sparser_consume_byte(ubx_sparser_ctx_t * ctx, uint8_t byte);


#endif /* INC_UBX_STREAM_PARSER_H_ */
