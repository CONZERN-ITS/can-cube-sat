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

const static uint8_t mavlink_system = CUBE_1_PCU;

mavlink_channel_t mavlink_claim_channel(void);

#endif /* INC_MAVLINK_HELP2_H_ */
