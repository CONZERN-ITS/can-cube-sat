/*
 * gps.c
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#include "ubx_stream_parser.h"

#include <string.h>
#include <assert.h>
#include <errno.h>


typedef enum ubx_sparser_state_t
{
	UBX_SPARSER_STATE_SYNC_SEARCH,
	UBX_SPARSER_STATE_HEADER_ACCUM,
	UBX_SPARSER_STATE_PACKET_ACCUM,
} ubx_sparser_state_t;


static int _check_packet_header(ubx_sparser_ctx_t * ctx)
{
	uint16_t length = ubx_packet_payload_size(ctx->pbuffer);

	if (length >= ctx->pbuffer_capacity)
		return -EMSGSIZE; // Пакет слишком большой

	return 0; // в остальном все ок
}


static int _check_whole_packet(ubx_sparser_ctx_t * ctx)
{
	const uint8_t actual_crc_a = ctx->pbuffer[ctx->pbuffer_size - 2];
	const uint8_t actual_crc_b = ctx->pbuffer[ctx->pbuffer_size - 1];
	const uint16_t actual_crc_16 = ubx_uint16crc_make(actual_crc_a, actual_crc_b);

	int crc_check_len = ctx->pbuffer_size - 2;		//контрольная сумма считается без crc полей
	uint16_t calculated_crc16 = ubx_packet_checksum(ctx->pbuffer, crc_check_len);

	if (calculated_crc16 == actual_crc_16)
		return 0;
	else
		return -EBADMSG;
}


static int _dispatch_packet(ubx_sparser_ctx_t * ctx)
{
	ubx_any_packet_t packet;

	int rc = ubx_parse_any_packet(ctx->pbuffer, &packet);
	if (0 == rc)
	{
		ctx->packet_callback(ctx->packet_callback_userarg, &packet);
	}
	return rc;
}


// ========================================
// Публичные функции
// ========================================
void ubx_sparser_reset(ubx_sparser_ctx_t * ctx)
{
	ctx->pbuffer_size = 0;
	ctx->expected_packet_size = 0;
	ctx->sw_buffer = 0;
	ctx->state = UBX_SPARSER_STATE_SYNC_SEARCH;
}


void ubx_sparser_set_packet_callback(ubx_sparser_ctx_t * ctx, ubx_sparser_packet_callback_t callback,
		void * userarg
){
	ctx->packet_callback = callback;
	ctx->packet_callback_userarg = userarg;

}


void ubx_sparser_set_pbuffer(ubx_sparser_ctx_t * ctx, uint8_t * pbuffer, uint16_t pbuffer_capacity)
{
//	assert(ctx->state == UBX_SPARSER_STATE_SYNC_SEARCH);

	ctx->pbuffer = pbuffer;
	ctx->pbuffer_capacity = pbuffer_capacity;
	ctx->pbuffer_size = 0;
}


void ubx_sparser_consume_byte(ubx_sparser_ctx_t * ctx, uint8_t byte)
{
	switch (ctx->state)
	{
		case UBX_SPARSER_STATE_SYNC_SEARCH:
			ctx->sw_buffer = (ctx->sw_buffer << 8 | byte);		//записываем синхрослово
			if (ctx->sw_buffer == UBX_SYNCWORD_VALUE)
			{
				ctx->pbuffer_size = 0;
				ctx->state = UBX_SPARSER_STATE_HEADER_ACCUM;
			}
		break;

		case UBX_SPARSER_STATE_HEADER_ACCUM:
			assert(ctx->pbuffer_size < ctx->pbuffer_capacity);

			ctx->pbuffer[ctx->pbuffer_size] = byte;
			ctx->pbuffer_size++;

			if (ctx->pbuffer_size < UBX_HEADER_SIZE)
				break;

			if (0 == _check_packet_header(ctx))
			{
				ctx->expected_packet_size = ubx_packet_payload_size(ctx->pbuffer) + UBX_HEADER_SIZE + UBX_CRC_SIZE;
				ctx->state = UBX_SPARSER_STATE_PACKET_ACCUM;
			}
			else
			{
				// FIXME: На самом деле синхрослово может быть
				// в тех байтах, которые мы уже накопили
				// и нужно бы проверить и тоже
				ctx->sw_buffer = 0;
				ctx->state = UBX_SPARSER_STATE_SYNC_SEARCH;
			}
		break;

		case UBX_SPARSER_STATE_PACKET_ACCUM:
			assert(ctx->pbuffer_size < ctx->pbuffer_capacity);

			ctx->pbuffer[ctx->pbuffer_size] = byte;
			ctx->pbuffer_size++;

			if (ctx->pbuffer_size < ctx->expected_packet_size)
				break;

			if (0 == _check_whole_packet(ctx))
			{
				_dispatch_packet(ctx);
			}

			ctx->sw_buffer = 0;
			ctx->state = UBX_SPARSER_STATE_SYNC_SEARCH;
		break;
	}; // case
}

