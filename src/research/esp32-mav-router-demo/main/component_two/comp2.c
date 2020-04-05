/*
 * comp2.c
 *
 *  Created on: 6 апр. 2020 г.
 *      Author: snork
 */


#include "comp2.h"

#include <time.h>


its_c2_rt_endpoint_t its_c2_rt_endpoint = { 0 };


//! Этот компонент будет просто считать пакеты, которые ходят по системе
static struct {
	int good_messages;
	int bad_messages;
	int ugly_messages;
} _counters;


//! Эту функцию вызывают всякие другие таски, но нам как-то пофиг...

static int accept_message(its_rt_endpoint_t * self_, const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg
){
	(void)self_;

	switch (msg->msgid)
	{
	case MAVLINK_MSG_ID_GPS_UBX_NAV_TIMEGPS:
	case MAVLINK_MSG_ID_GPS_UBX_NAV_SOL:
		// Это будут хорошие сообщения
		_counters.good_messages++;
		break;

	case MAVLINK_MSG_ID_HEARTBEAT:
		// Это плохие
		_counters.bad_messages++;
		break;

	default:
		// а это противные
		_counters.ugly_messages++;
		break;
	}

	return 0;
}


void its_c2_init(void)
{
	its_c2_rt_endpoint.base.acceptor = accept_message;
}

