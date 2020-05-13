/*
 * mav_packers.c
 *
 *  Created on: 22 апр. 2020 г.
 *      Author: developer
 */

#include "mav_packet.h"

#include <mavlink/its/mavlink.h>

#include "drivers/time_svc/time_svc.h"
#include "drivers/uplink.h"
#include "state.h"

int _mavlink_sins_isc(stateSINS_isc_t * state_isc)
{
		mavlink_sins_isc_t msg_sins_isc;
		msg_sins_isc.time_s = state_isc->tv.tv_sec;
		msg_sins_isc.time_us = state_isc->tv.tv_usec;

		for (int i = 0; i < 3; i++)
		{
			msg_sins_isc.accel[i] = state_isc->accel[i];
			msg_sins_isc.compass[i] = state_isc->magn[i];
			msg_sins_isc.quaternion[i] = state_isc->quaternion[i];
		}
		msg_sins_isc.quaternion[3] = state_isc->quaternion[3];


		mavlink_message_t msg;
		mavlink_msg_sins_isc_encode(0, 0, &msg, &msg_sins_isc);
		int error = uplink_write_mav(&msg);
		return error;
}

int _mavlink_timestamp()
{
	mavlink_timestamp_t msg_timestamp;
	struct timeval tv;

	time_svc_world_get_time(&tv);
	msg_timestamp.time_s = tv.tv_sec;
	msg_timestamp.time_us = tv.tv_usec;
	msg_timestamp.time_base = 0;		//FIXME: исправить

	mavlink_message_t msg;
	mavlink_msg_timestamp_encode(0, 0, &msg, &msg_timestamp);
	int error = uplink_write_mav(&msg);
	return error;
}


void _on_gps_packet(void * arg, const ubx_any_packet_t * packet)
{
	volatile ubx_pid_t pid = packet->pid;
	switch (pid)
	{
	case UBX_PID_NAV_SOL:
		{
			struct timeval tv;
			time_svc_world_get_time(&tv);

			mavlink_gps_ubx_nav_sol_t msg_gps_ubx_nav_sol;
			msg_gps_ubx_nav_sol.time_s = tv.tv_sec;
			msg_gps_ubx_nav_sol.time_us = tv.tv_usec;
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
			mavlink_msg_gps_ubx_nav_sol_encode(0, 0, &msg, &msg_gps_ubx_nav_sol);
			uplink_write_mav(&msg);
			break;
		}

	}
}
