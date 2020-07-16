/*
 * mavlink_help2.h
 *
 *  Created on: Jul 16, 2020
 *      Author: sereshotes
 */

#ifndef INC_MAVLINK_HELP2_H_
#define INC_MAVLINK_HELP2_H_


#include "mavlink/its/mavlink.h"

#include "assert.h"

const static mavlink_system_t mavlink_system = {CUBE_1, CUBE_1_PCU};

static mavlink_channel_t mavlink_claim_channel(void) {
    static int channel = -1;
    channel++;
    assert(channel < MAVLINK_COMM_NUM_BUFFERS);
    return (mavlink_channel_t) channel;
}



#endif /* INC_MAVLINK_HELP2_H_ */
