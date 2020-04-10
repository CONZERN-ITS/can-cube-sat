/*
 * ubx_packets.c
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#include "ubx_packet_parser.h"

#include <errno.h>


uint16_t ubx_packet_pid(const uint8_t * packet_header)
{
	return ((uint16_t)packet_header[1] << 8) | packet_header[0];
}


uint16_t ubx_packet_payload_size(const uint8_t * packet_header)
{
	return ((uint16_t)packet_header[3] << 8) | (packet_header[2]);
}


uint16_t ubx_packet_payload_expected_size(ubx_pid_t pid)
{
	switch (pid)
	{
	case UBX_PID_NAV_SOL:
		return 52;

	case UBX_PID_TIM_TP:
		return 16;

	case UBX_PID_NAV_TIMEGPS:
		return 16;

	default:
		return 0;
	}
}


static ubx_pid_t _make_pid(uint8_t class, uint8_t id)
{
	return ((uint16_t)class << 8) | id;
}



static uint32_t _read_u32(const uint8_t * data)
{
	return	  (uint32_t)data[3] << 3*8
			| (uint32_t)data[2] << 2*8
			| (uint32_t)data[1] << 1*8
			| (uint32_t)data[0] << 0*8
	;
}


static uint32_t _read_i32(const uint8_t * data)
{
	uint32_t buffer = _read_u32(data);
	return *(int16_t*)&buffer;
}


static uint32_t _read_u16(const uint8_t * data)
{
	return	  (uint16_t)data[1] << 1*8
			| (uint16_t)data[0] << 0*8
	;
}


//TODO: дописать
static void _ubx_parse_nav_sol(const uint8_t * payload, ubx_any_packet_t * packet_)
{
	// обрезание пакета до 48 байт т.к. дальше идут зарезервированные поля
	ubx_navsol_packet_t * packet = &packet_->packet.navsol;

	packet->i_tow		= _read_u32(payload + 0);
	packet->f_tow		= _read_i32(payload + 4);
	packet->week		= _read_u16(payload + 8);
	packet->gps_fix		= *(payload + 10);
	packet->flags		= *(payload + 11);
	packet->pos_ecef[0] = _read_i32(payload + 12);
	packet->pos_ecef[1] = _read_i32(payload + 16);
	packet->pos_ecef[2] = _read_i32(payload + 20);
	packet->p_acc		= _read_u32(payload + 24);
	packet->vel_ecef[0] = _read_i32(payload + 28);
	packet->vel_ecef[1] = _read_i32(payload + 32);
	packet->vel_ecef[2] = _read_i32(payload + 36);
	packet->s_acc		= _read_u32(payload + 40);
	packet->p_dop		= _read_u16(payload + 44);
	// 1 байт резерва
	packet->num_sv		= *(payload + 47);
	// 4 байта в резерве
}


// время GPS
static void _ubx_parse_tim_tp(const uint8_t * payload, ubx_any_packet_t * packet_)
{
	ubx_timtp_packet_t * packet = &packet_->packet.timtp;

	packet->tow_ms = _read_u32(payload);
	packet->week = _read_u16(payload + 12);		// +12 т.к. время в неделях с 12 по 14 байт
}


static void _ubx_parse_nav_timegps(const uint8_t * payload, ubx_any_packet_t * packet_)
{
	ubx_gpstime_packet_t * packet = &packet_->packet.gpstime;

	packet->tow_ms = _read_u32(payload);
	packet->week = _read_u16(payload + 8);		// +8 т.к. время в неделях с 8 по 10 байт
}


static void _ubx_parse_ack(const uint8_t * payload, ubx_any_packet_t * packet_)
{
	ubx_ack_packet_t * packet = &packet_->packet.ack;

	packet->packet_pid = _make_pid(payload[0], payload[1]);
}


static void _ubx_parse_nack(const uint8_t * payload, ubx_any_packet_t * packet_)
{
	ubx_nack_packet_t * packet = &packet_->packet.nack;

	packet->packet_pid = _make_pid(payload[0], payload[1]);
}


int ubx_parse_any_packet(const uint8_t * packet_start, ubx_any_packet_t * packet)
{
	uint16_t pid = ubx_packet_pid(packet_start);
	const uint8_t * payload_start = packet_start + UBX_HEADER_SIZE;
	int rc = 0;

	switch (pid)
	{
	case UBX_PID_NAV_SOL:
		_ubx_parse_nav_sol(payload_start, packet);
		break;

	case UBX_PID_NAV_TIMEGPS:
		_ubx_parse_nav_timegps(payload_start, packet);
		break;

	case UBX_PID_TIM_TP:
		_ubx_parse_tim_tp(payload_start, packet);
		break;

	case UBX_PID_CFG_ACK:
		_ubx_parse_ack(payload_start, packet);
		break;

	case UBX_PID_CFG_NACK:
		_ubx_parse_nack(payload_start, packet);
		break;

	default:
		rc = -ENOSYS;
		break;
	};

	packet->pid = pid;
	return rc;
}

