/*
 * mavlink_help2.h
 *
 *  Created on: 23 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef COMPONENTS_MAVLINK_COMP_MAVLINK_HELP2_H_
#define COMPONENTS_MAVLINK_COMP_MAVLINK_HELP2_H_

#include "mavlink/its/mavlink.h"
#include "init_helper.h"

#if ITS_WIFI_SERVER
const static mavlink_system_t mavlink_system = {CUBE_1, CUBE_1_BCU};
#else
const static mavlink_system_t mavlink_system = {CUBE_2, CUBE_2_PFU};
#endif

/*
 * Для упращения управления каналами
 */
static mavlink_channel_t mavlink_claim_channel(void) {
	static int channel = -1;
	channel++;
	return (mavlink_channel_t) channel;
}


#endif /* COMPONENTS_MAVLINK_COMP_MAVLINK_HELP2_H_ */
