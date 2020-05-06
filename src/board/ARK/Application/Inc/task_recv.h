/*
 * task_recv.h
 *
 *  Created on: Apr 28, 2020
 *      Author: sereshotes
 */

#ifndef INC_TASK_RECV_H_
#define INC_TASK_RECV_H_

#include "mavlink/its/mavlink.h"

#define TRECV_MAX_CALLBACK_COUNT 5


void task_recv_init(void *arg);
void task_recv_update(void *arg);


void trecv_add_callbac(void (*f)(const mavlink_message_t *msg));

#endif /* INC_TASK_RECV_H_ */
