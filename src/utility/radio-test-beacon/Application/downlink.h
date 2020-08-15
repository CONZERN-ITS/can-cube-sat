/*
 * mav_packet.h
 *
 *  Created on: 22 апр. 2020 г.
 *      Author: developer
 */

#ifndef MAV_PACKET_H_
#define MAV_PACKET_H_

#include "gps/ubx_packet_parser.h"


void downlink_init(void);

void downlink_gps_packet(const ubx_any_packet_t * packet);

#endif /* MAV_PACKET_H_ */
