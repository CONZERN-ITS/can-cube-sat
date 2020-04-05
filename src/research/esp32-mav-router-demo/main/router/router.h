/*
 * router.h
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#ifndef MAIN_ROUTER_ROUTER_H_
#define MAIN_ROUTER_ROUTER_H_

#include "../mavlink/its/mavlink.h"

#include "rt_sender_ctx.h"

//! Функция - маршрутизатор
/*! Эта функция отправляет пакеты всем абонентам. А абоненты решают нужны им эти пакеты или нет */
void its_rt_route(const its_rt_sender_ctx_t * sender_ctx, const mavlink_message_t * msg);


#endif /* MAIN_ROUTER_ROUTER_H_ */
