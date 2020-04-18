/*
 * imi.h
 *
 *  Created on: 27 мар. 2020 г.
 *      Author: sereshotes
 */

#ifndef MAIN_INC_IMI_H_
#define MAIN_INC_IMI_H_


#include <stdint.h>

#include "init_helper.h"
#include "my_i2c.h"


#define IMI_WAIT_DELAY 100 //ms
#define IMI_CYCLE_DELAY 10 //ms

typedef enum imi_cmd_t
{
    IMI_CMD_NONE = 0x00,
	IMI_CMD_GET_SIZE = 0x01,
	IMI_CMD_GET_PACKET = 0x02,
	IMI_CMD_SET_PACKET = 0x04,
} imi_cmd_t;


typedef struct {
	i2c_port_t i2c_port;
	uint8_t address;
	int timeout;
} imi_t;

extern imi_t imi_handler[ITS_IMI_DEVICE_COUNT];

int imi_msg_recieve(imi_t *himi, uint8_t *data, uint16_t *size);

int imi_send_cmd(imi_t *himi, imi_cmd_t cmd);

int imi_msg_send(imi_t *himi, uint8_t *data, uint16_t size);

void imi_msg_rcv_task(void *pv);

void IRAM_ATTR imi_i2c_int_isr_handler(void* arg);

void imi_set_alloc_callback(uint8_t* (*alloc)(uint16_t size));

void imi_set_save_callback(void (*save)(uint8_t *data, uint16_t size));

void imi_init(void);

#endif /* MAIN_INC_IMI_H_ */
