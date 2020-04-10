 /*
 * ubx_packets.h
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#ifndef INC_UBX_PACKET_PARSER_H_
#define INC_UBX_PACKET_PARSER_H_


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
	UBX_PID_CFG_NACK = 0x0500,
	UBX_PID_CFG_ACK  = 0x0501,
} ubx_pid_t;


//! тип FIX
typedef enum ubx_fix_type_t
{
	//! No Fix (нет фиксации)
	UBX_FIX_TYPE__NO_FIX = 0x00,
	//! только Dead Reckoning
	UBX_FIX_TYPE__DEAD_RECKONING = 0x01,
	//! 2D-Fix
	UBX_FIX_TYPE__2D = 0x02,
	//! 3D-Fix
	UBX_FIX_TYPE__3D = 0x03,
	//! комбинация GNSS + dead reckoning
	UBX_FIX_TYPE__COMBO = 0x04,
	//! только фиксация времени
	UBX_FIX_TYPE__TIME_ONLY = 0x05,

	// Прочие значения зарезервированы
} ubx_fix_type_t;


//! Битовые флаги из поля flags сообщения ubx_nav_sol_packet_t
typedef enum ubx_nav_sol_flags_t
{
	//!  фиксация в определенных пределах
	UBX_NAVSOL_FLAGS__GPS_FIX_OK	= 0x01 << 0,
	//! использовалось DGPS
	UBX_NAVSOL_FLAGS__DIFF_SOL_IN	= 0x01 << 1,
	//! достоверный номер недели GPS
	UBX_NAVSOL_FLAGS__WKN_SET		= 0x01 << 2,
	//!  достоверно время недели GPS (iTOW и fTOW)
	UBX_NAVSOL_FLAGS__TOW_SET		= 0x01 << 3
} ubx_nav_sol_flags_t;


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


typedef struct ubx_navsol_packet_t
{
	//! Время недели GPS для эпохи навигации
	uint32_t i_tow;
	//! Дробная часть iTOW (диапазон ±500000).
	int32_t f_tow;
	//! Номер недели эпохи навигации
	uint16_t week;
	//! Тип фиксации позиции GPS. Значения описаны в ubx_fix_type_t
	uint8_t gps_fix;
	//! Флаги состояния фиксации. Значения описаны в ubx_nav_sol_flags_t
	uint8_t flags;
	//! Координаты в ECEF
	int32_t pos_ecef[3];
	//! Оценка точности положения в ECEF
	uint32_t p_acc;
	//! Скорость в ECEF
	int32_t vel_ecef[3];
	//! Оценка точности скорости в ECEF
	uint32_t s_acc;
	//! DOP позиционирования
	uint16_t p_dop;
	// Количество спутников
	uint8_t num_sv;
} ubx_navsol_packet_t;


//! Подтверждение получения валидного конфигурационного пакета
typedef struct ubx_ack_packet_t
{
	//! Идентификатор пакета
	ubx_pid_t packet_pid;
} ubx_ack_packet_t;


//! Сообщение об ошибке в полученном конфигурационном пакете
typedef struct ubx_nack_packet_t
{
	//! Идентификатор пакета
	ubx_pid_t packet_pid;
} ubx_nack_packet_t;


//! Структура, включающая данные любого пакета
typedef struct ubx_any_packet_t
{
	ubx_pid_t pid;

	union
	{
		ubx_gpstime_packet_t gpstime;
		ubx_timtp_packet_t timtp;
		ubx_navsol_packet_t navsol;
		ubx_ack_packet_t ack;
		ubx_nack_packet_t nack;
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


#endif /* INC_UBX_PACKET_PARSER_H_ */
