/*
 * mavlink_help2.c
 *
 *  Created on: 17 сент. 2020 г.
 *      Author: snork
 */

#include "mavlink_help2.h"

static int channel = -1;

mavlink_channel_t mavlink_claim_channel(void) {
	channel++;
	assert(channel < MAVLINK_COMM_NUM_BUFFERS);
	return (mavlink_channel_t) channel;
}
