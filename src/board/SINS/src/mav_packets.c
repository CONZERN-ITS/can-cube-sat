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

#include "drivers/temp/analog.h"

#include "diag/Trace.h"

#define SYSTEM_ID CUBE_1_SINS
#define COMPONENT_ID COMP_ANY_0

int mavlink_sins_isc(stateSINS_isc_t * state_isc)
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
		mavlink_msg_sins_isc_encode(SYSTEM_ID, COMPONENT_ID, &msg, &msg_sins_isc);
		int error = uplink_write_mav(&msg);
		return error;
}


int mavlink_timestamp(void)
{
	mavlink_timestamp_t msg_timestamp;
	struct timeval tv;

	time_svc_world_get_time(&tv);
	msg_timestamp.time_s = tv.tv_sec;
	msg_timestamp.time_us = tv.tv_usec;
	msg_timestamp.time_base = (uint8_t)time_svc_timebase();

	mavlink_message_t msg;
	mavlink_msg_timestamp_encode(SYSTEM_ID, COMPONENT_ID, &msg, &msg_timestamp);
	int error = uplink_write_mav(&msg);
	return error;
}


void on_gps_packet(void * arg, const ubx_any_packet_t * packet)
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
			mavlink_msg_gps_ubx_nav_sol_encode(SYSTEM_ID, COMPONENT_ID, &msg, &msg_gps_ubx_nav_sol);
			uplink_write_mav(&msg);
			break;
		}

	default:
		break;

	}
}


int own_temp_packet(void)
{
	int error = 0;

	struct timeval tv;
	time_svc_world_get_time(&tv);

	mavlink_own_temp_t own_temp_msg;

	const int oversampling = 10;
	uint32_t raw_sum = 0;
	uint16_t raw;
	for (int i = 0; i < oversampling; i++)
	{
		error = analog_get_raw(ANALOG_TARGET_INTEGRATED_TEMP, &raw);
		if (0 != error)
			return error;
		raw_sum += raw;
	}

		raw = raw_sum / oversampling;

		float mv = raw * 3300.0f / 0x0FFF;
		float temp = (mv - INTERNAL_TEMP_V25) / INTERNAL_TEMP_AVG_SLOPE + 25;

		own_temp_msg.time_s = tv.tv_sec;
		own_temp_msg.time_us = tv.tv_usec;
		own_temp_msg.deg = temp;

		mavlink_message_t msg;
		mavlink_msg_own_temp_encode(SYSTEM_ID, COMPONENT_ID, &msg, &own_temp_msg);
		uplink_write_mav(&msg);

		return 0;
}


int mavlink_errors_packet(void)
{
	int error = 0;

	struct timeval tv;
	time_svc_world_get_time(&tv);

	mavlink_sins_errors_t errors_msg;

	errors_msg.time_s = tv.tv_sec;
	errors_msg.time_us = tv.tv_usec;

	errors_msg.analog_sensor_init_error = error_system.analog_sensor_init_error;

	errors_msg.gps_init_error = error_system.gps_init_error;
	errors_msg.gps_config_error = error_system.gps_config_error;
	errors_msg.gps_uart_init_error = error_system.gps_uart_error;
	errors_msg.gps_reconfig_counter = error_system.gps_reconfig_counter;

	errors_msg.mems_i2c_error = error_system.mems_i2c_error;
	errors_msg.mems_i2c_error_counter = error_system.mems_i2c_error_counter;

	errors_msg.lsm6ds3_error = error_system.lsm6ds3_error;
	errors_msg.lsm6ds3_error_counter = error_system.lsm6ds3_error_counter;

	errors_msg.lis3mdl_error = error_system.lis3mdl_error;
	errors_msg.lis3mdl_error_counter = error_system.lis3mdl_error_counter;

	errors_msg.rtc_error = error_system.rtc_error;

	errors_msg.timers_error = error_system.timers_error;

	errors_msg.uart_transfer_init_error = error_system.uart_transfer_init_error;
	errors_msg.uart_transfer_error = error_system.uart_transfer_error;

	errors_msg.reset_counter = error_system.reset_counter;

	mavlink_message_t msg;
	mavlink_msg_sins_errors_encode(SYSTEM_ID, COMPONENT_ID, &msg, &errors_msg);
	uplink_write_mav(&msg);

	return 0;
}
