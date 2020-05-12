/*
 * imi.h
 *
 *  Created on: 27 мар. 2020 г.
 *      Author: sereshotes
 */
/*	MANUAL
 *
 * 1. Set up needed systems:
 * 	a. gpio interrupts through gpio_install_isr_service(0);
 * 	b. i2c port
 * 2. Install imi driver through imi_install setting all
 * fields of imi_config_t structure. Also you can use imi_install_static
 * if you don't want the driver to allocate memory for address in heap.
 * In that case you have to provide memory that will be used until
 * driver is stopped.
 * 3. Add addresses of all devices, that will pull i2c_int line.
 * 4. Start driver.
 */

#ifndef MAIN_INC_IMI_H_
#define MAIN_INC_IMI_H_


#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "init_helper.h"
#include "my_i2c.h"


#define IMI_WAIT_DELAY 100 //ms
#define IMI_CYCLE_DELAY 100 //ms
#define IMI_COUNT 1

typedef int imi_port_t;

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

//extern imi_t imi_handler[ITS_IMI_DEVICE_COUNT];

typedef struct {
	i2c_port_t i2c_port;
	int i2c_int;
	int address_count;
	int ticksToWaitForOne;
	void (*save)(uint8_t *data, uint16_t size);
	uint8_t *(*alloc)(uint16_t size);
} imi_config_t;


/*
int imi_msg_recieve(imi_t *himi, uint8_t *data, uint16_t *size);

int imi_send_cmd(imi_t *himi, imi_cmd_t cmd);

int imi_msg_send(imi_t *himi, uint8_t *data, uint16_t size);
*/
void imi_msg_rcv_task(void *pv);

void imi_set_alloc_callback(uint8_t* (*alloc)(uint16_t size));

void imi_set_save_callback(void (*save)(uint8_t *data, uint16_t size));

void imi_install(imi_config_t *cfg, imi_port_t port);

void imi_install_static(imi_config_t *cfg, imi_port_t port, uint8_t *addrs);

void imi_start(imi_port_t port);

void imi_add_address(imi_port_t port, uint8_t address);

int imi_send(int port, uint8_t address, uint8_t *data, uint16_t size, TickType_t ticksToWait);

int imi_send_all(int port, uint8_t *data, uint16_t size, TickType_t ticksToWaitForOne);

void imi_init(void);

#endif /* MAIN_INC_IMI_H_ */
/*
 * imi.h
 *
 *  Created on: 27 мар. 2020 г.
 *      Author: sereshotes
 */



