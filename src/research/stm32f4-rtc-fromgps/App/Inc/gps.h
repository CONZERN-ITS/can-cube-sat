/*
 * gps.h
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#include "ubx_stream_parser.h"

void gps_setup(ubx_sparser_packet_callback_t callback, void * callback_arg);

int gps_poll(void);
