 /*
 * ubx_packets.h
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#ifndef INC_UBX_PACKETS_H_
#define INC_UBX_PACKETS_H_


#include <stdint.h>

//! Синхрослово пакета ubx
#define UBX_SYNCWORD_VALUE 0xb562

//! Длина заголовка пакета UBX
#define UBX_HEADER_SIZE 4

//! Длина контрольной суммы UBX пакета
#define UBX_CRC_SIZE 2


//! Идентификатор пакета
/*! Состоит из class и id пакета */
typedef enum ubx_pid_t
{
	UBX_PID_NAV_SOL = 0x0601,
	UBX_PID_TIM_TP  = 0x010d,
	UBX_PID_NAV_TIMEGPS = 0x2001,
} ubx_pid_t;


//! Пакет gpstime
typedef struct ubx_gpstime_packet_t
{
	uint16_t week;
	uint32_t tow_ms;
} ubx_gpstime_packet_t;


//! Пакет timtp
typedef struct ubx_timtp_packet_t
{
	uint16_t week;
	uint32_t tow_ms;
} ubx_timtp_packet_t;


//! Структура, включающая данные любого пакета
typedef struct ubx_any_packet_t
{
	ubx_pid_t pid;

	union
	{
		ubx_gpstime_packet_t gpstime;
		ubx_timtp_packet_t timtp;
	} packet;
} ubx_any_packet_t;

//! Извлечение идентификатора пакета из заголовка собщения
uint16_t ubx_packet_pid(const uint8_t * packet_header);

//! Извлечение длины пакета из заголовка
uint16_t ubx_packet_payload_size(const uint8_t * packet_header);

//! Длина пакета, которая должна у него быть согласно протоколу
uint16_t ubx_packet_payload_expected_size(ubx_pid_t pid);


//! Разбор тела пакета
int ubx_parse_any_packet(const uint8_t * packet_start, ubx_any_packet_t * packet);


#endif /* INC_UBX_PACKETS_H_ */
