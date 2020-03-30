/*
 * gps.h
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#ifndef DRIVERS_GPS_H_
#define DRIVERS_GPS_H_

#define GPS_BUFFER_SIZE 500

#define UBX_SYNCWORD_VALUE 0xb562
#define UBX_HEADER_LEN 4

//		CLASS и ID используемых сообщений		//
#define CLASS_ID_UBX_NAV_SOL		0x0601
#define CLASS_ID_UBX_TIM_TP			0x010d
#define CLASS_ID_UBX_NAV_TIMEGPS	0x2001

#define LEN_UBX_NAV_SOL				52
#define LEN_UBX_TIM_TP				16
#define LEN_UBX_NAV_TIMEGPS			16


#include "state.h"


typedef enum state_t
{
	GPS_STATE_SYNC_SEARCH,
	GPS_STATE_HEADER_ACCUM,
	GPS_STATE_PACKET_ACCUM,
} state_t;


typedef struct gps_ctx_t
{
	state_t state;					//состояние пакета
	uint16_t sw_buffer;				//синхрослово
	uint8_t packet_buffer[100];		//пакет
	size_t bytes_accum;				//кол-во принятых байт
	size_t expected_packet_size;	//ожидаемая длина пакета
} gps_ctx_t;


extern gps_ctx_t gps;


extern void uartGPSInit(UART_HandleTypeDef * uart);
extern void gps_init(gps_ctx_t * ctx);
extern void gps_consume_byte(gps_ctx_t * ctx, uint8_t byte);
extern int read_gps_buffer();


#endif /* DRIVERS_GPS_H_ */
