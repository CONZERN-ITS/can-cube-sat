#include "ublox-cfg-sender.h"

#define UBX_SYNCWORD (0x62B5) // На самом деле 0xB5 а потом 0x62, но у нас litle endian, поэтому все наоборот


inline static uint16_t _msg_len(const uint8_t * msg_body_start)
{
	const uint16_t retval = ((uint16_t)msg_body_start[2] << 8) | (msg_body_start[3] << 0);
	return retval;
}


static uint16_t _checksum(const uint8_t * data, size_t data_size8)
{
	uint8_t CK_A = 0;
	uint8_t CK_B = 0;

	for (size_t i = 0; i < data_size8; i++)
	{
		CK_A = CK_A + data[i];
		CK_B = CK_B + CK_A;
	}

	return ((uint16_t)CK_A & 0xFF) | ((uint16_t)CK_B << 8);
}


static int _send_one(ubx_cfg_sender_ctx_t * ctx, const uint8_t * msg_body_start)
{
	static const uint16_t syncword = UBX_SYNCWORD;
	const uint16_t msg_len = _msg_len(msg_body_start);
	const uint16_t checksum = _checksum(msg_body_start, msg_len);

	int rv = ctx->uart_write(ctx->user_arg, (uint8_t*)&syncword, sizeof(syncword));
	if (rv < 0)
		return rv;

	rv = ctx->uart_write(ctx->user_arg, msg_body_start, msg_len);
	if (rv < 0)
		return rv;

	rv = ctx->uart_write(ctx->user_arg, (uint8_t*)&checksum, sizeof(checksum));
	return rv;
}


int ubx_cfg_sender_send_cfg(ubx_cfg_sender_ctx_t * ctx)
{
	const uint8_t ** msg_buffer;

	for (msg_buffer = ctx->cfg_msg_defs; *msg_buffer != 0; msg_buffer++)
	{
		int rv = _send_one(ctx, *msg_buffer);
		if (rv < 0)
			return rv;
	}

	return 0;
}
