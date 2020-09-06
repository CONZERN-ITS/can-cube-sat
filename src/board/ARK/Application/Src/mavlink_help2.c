/*
 * mavlink_help2.c
 *
 *  Created on: Sep 6, 2020
 *      Author: snork
 */

#include "mavlink_help2.h"

mavlink_channel_t mavlink_claim_channel(void) {
    static int channel = -1;
    channel++;
    assert(channel < MAVLINK_COMM_NUM_BUFFERS);
    return (mavlink_channel_t) channel;
}
