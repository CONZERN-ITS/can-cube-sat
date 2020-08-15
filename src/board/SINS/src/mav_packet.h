/*
 * mav_packet.h
 *
 *  Created on: 22 апр. 2020 г.
 *      Author: developer
 */

#ifndef MAV_PACKET_H_
#define MAV_PACKET_H_

#include "state.h"
#include <mavlink/its/mavlink.h>

extern int mavlink_sins_isc(stateSINS_isc_t * state_isc);
extern int mavlink_timestamp();
extern void on_gps_packet(void * arg, const ubx_any_packet_t * packet);
extern int own_temp_packet();

#endif /* MAV_PACKET_H_ */
