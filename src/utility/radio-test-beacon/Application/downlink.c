/*
 * mav_packers.c
 *
 *  Created on: 22 апр. 2020 г.
 *      Author: developer
 */

#include "downlink.h"

#include <mavlink/its/mavlink.h>

#include <stm32f1xx_hal.h>

extern UART_HandleTypeDef huart3;
static UART_HandleTypeDef * const husart_radio = &huart3;

#define SYSTEM_ID CUBE_1_SINS
#define COMPONENT_ID COMP_ANY_0


static void _downlink_mavpacket(const mavlink_message_t * packet)
{
	static uint8_t packet_buffer[MAVLINK_MAX_PACKET_LEN];

	uint16_t len = mavlink_msg_to_send_buffer(packet_buffer, packet);
	HAL_UART_Transmit(husart_radio, packet_buffer, len, HAL_MAX_DELAY);
}


void downlink_init(void)
{
	// Все настроено кубемх
}


void downlink_gps_packet(const ubx_any_packet_t * packet)
{
	volatile ubx_pid_t pid = packet->pid;
	switch (pid)
	{
	case UBX_PID_NAV_SOL:
		{
			mavlink_gps_ubx_nav_sol_t msg_gps_ubx_nav_sol = {0};
			msg_gps_ubx_nav_sol.time_s = 0;
			msg_gps_ubx_nav_sol.time_us = 0;
			msg_gps_ubx_nav_sol.iTOW = packet->packet.navsol.i_tow;
			msg_gps_ubx_nav_sol.fTOW = packet->packet.navsol.f_tow;
			msg_gps_ubx_nav_sol.week = packet->packet.navsol.week;
			msg_gps_ubx_nav_sol.gpsFix = packet->packet.navsol.gps_fix;
			msg_gps_ubx_nav_sol.flags = packet->packet.navsol.flags;
			msg_gps_ubx_nav_sol.ecefX = packet->packet.navsol.pos_ecef[0];
			msg_gps_ubx_nav_sol.ecefY = packet->packet.navsol.pos_ecef[1];
			msg_gps_ubx_nav_sol.ecefZ = packet->packet.navsol.pos_ecef[2];
			msg_gps_ubx_nav_sol.pAcc = packet->packet.navsol.p_acc;
			msg_gps_ubx_nav_sol.ecefVX = packet->packet.navsol.vel_ecef[0];
			msg_gps_ubx_nav_sol.ecefVY = packet->packet.navsol.vel_ecef[1];
			msg_gps_ubx_nav_sol.ecefVZ = packet->packet.navsol.vel_ecef[2];
			msg_gps_ubx_nav_sol.sAcc = packet->packet.navsol.s_acc;
			msg_gps_ubx_nav_sol.pDOP = packet->packet.navsol.p_dop;
			msg_gps_ubx_nav_sol.numSV = packet->packet.navsol.num_sv;

			mavlink_message_t msg;
			mavlink_msg_gps_ubx_nav_sol_encode(SYSTEM_ID, COMPONENT_ID, &msg, &msg_gps_ubx_nav_sol);
			_downlink_mavpacket(&msg);
			break;
		}

	default:
		break;

	}
}

