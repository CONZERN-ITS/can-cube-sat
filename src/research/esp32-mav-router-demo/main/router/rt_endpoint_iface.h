/*
 * router.h
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#ifndef MAIN_RT_ENDPOINT_IFACE_H_
#define MAIN_RT_ENDPOINT_IFACE_H_

#include "../mavlink/its/mavlink.h"

#include "rt_sender_ctx.h"


struct its_rt_endpoint_t;
typedef struct its_rt_endpoint_t its_rt_endpoint_t;

//! Функция - акцептор. Это функции роутер показывает все пакеты, которые есть в системе
/*! А функция может с ними что-нибудь сделать */
typedef int (*its_router_endpoint_acceptor)(
		its_rt_endpoint_t * /*self*/,
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * /*msg*/
);


//! Структура эндпоинта маршрутизатора
/*! Эндпоит представляет собой абонента, который может принимать пакеты
  от кого-бы то ни было */
struct its_rt_endpoint_t
{
	//! Фукнция - приёмник данных этого эндпоинта
	its_router_endpoint_acceptor acceptor;
};



#endif /* MAIN_RT_ENDPOINT_IFACE_H_ */
