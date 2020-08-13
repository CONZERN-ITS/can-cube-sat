/*
 * router.h
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#ifndef MAIN_ROUTER_ROUTER_H_
#define MAIN_ROUTER_ROUTER_H_

#include "mavlink/its/mavlink.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "rt_sender_ctx.h"

//! Функция - маршрутизатор
/*! Эта функция отправляет пакеты всем абонентам. А абоненты решают нужны им эти пакеты или нет */
void its_rt_route_from_isr(
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg
);

typedef struct its_rt_task_identifier {
	QueueHandle_t queue;
	char name[16];
}its_rt_task_identifier;

void its_rt_route(
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg,
		TickType_t ticksToWaitForOne
);
int its_rt_register(int msg_id, its_rt_task_identifier task_id);
int its_rt_register_for_all(its_rt_task_identifier task_id);
int its_rt_unregister(int msgid, its_rt_task_identifier task_id);
int its_rt_unregister_for_all(its_rt_task_identifier task_id);
void its_rt_uninit();


#endif /* MAIN_ROUTER_ROUTER_H_ */
