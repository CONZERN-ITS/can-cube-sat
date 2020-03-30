/*
 * ark.h
 *
 *  Created on: 27 мар. 2020 г.
 *      Author: sereshotes
 */

#ifndef MAIN_INC_ARK_H_
#define MAIN_INC_ARK_H_


#include <stdint.h>

#include "my_i2c.h"

struct Message {
	uint16_t size;
	uint8_t *packet;
};

typedef enum ark_cmd_t
{
    ARK_CMD_NONE = 0x00,
	ARK_CMD_GET_SIZE = 0x01,
	ARK_CMD_GET_PACKET = 0x02,
	ARK_CMD_SET_PACKET = 0x04,
} ark_cmd_t;


typedef struct {
	i2c_port_t i2c_port;
	uint8_t address;
	int timeout;
} ark_t;

int ark_msg_recieve(ark_t *hark, uint8_t *data, uint16_t size);

int ark_send_cmd(ark_t *hark, ark_cmd_t cmd);

int ark_msg_send(ark_t *hark, uint8_t *data, uint16_t size);

#endif /* MAIN_INC_ARK_H_ */
