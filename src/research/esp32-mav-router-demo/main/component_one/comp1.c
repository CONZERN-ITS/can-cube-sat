/*
 * task.c
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#include "comp1.h"

#include "../router/router.h"


#define ITS_COMP1_MAVCOMP_ID 10



its_c1_rt_endpoint_t its_c1_rt_endpoint = { 0 };



//! Фильтрация входящих сообщений и их предобработка
/*! Эту функцию вызывают другие таски, поэтому для работы с контекстом нашего таска - нужно
 *  как-то синхронизироваться. Мы будем использовать очередь */
static int accept_message(its_rt_endpoint_t * self_, const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg
){
	its_c1_rt_endpoint_t * self = (its_c1_rt_endpoint_t * )self_;

	switch (msg->msgid)
	{
	case MAVLINK_MSG_ID_GPS_UBX_NAV_TIMEGPS:
	case MAVLINK_MSG_ID_GPS_UBX_NAV_SOL:
		// Эти сообщения мы хотим
		if (sender_ctx->from_isr)
		{
			// Если нас дернули из прерывания, то ведем себя вот так
			BaseType_t higher_prio_woken;
			xQueueSendFromISR(self->task_input_queue, msg, &higher_prio_woken);
			if (higher_prio_woken)
				portYIELD_FROM_ISR();
		}
		else
		{
			// Если не из прерывания - не стесняемся
			// FIXME: возможно оптравитель захочет нам сказать, что стеснятся как раз надо?
			xQueueSend(self->task_input_queue, msg, portMAX_DELAY);
		}
		break;

	default:
		//Все остальные не хотим
		break;
	}

	return 0;
}


//! Обработка пришедшего сообщения
static void process_message(const mavlink_message_t * msg)
{
	switch (msg->msgid)
	{
	case MAVLINK_MSG_ID_GPS_UBX_NAV_SOL:
		// Пришли данные навигации
		{
			int32_t pos[3];
			pos[0] = mavlink_msg_gps_ubx_nav_sol_get_ecefX(msg);
			pos[1] = mavlink_msg_gps_ubx_nav_sol_get_ecefY(msg);
			pos[2] = mavlink_msg_gps_ubx_nav_sol_get_ecefZ(msg);

			// Что-нибудь с ними делаем
			(void)pos;
		}
		break;

	default:
		// Все другие сообщения нам не интересны
		break;
	}
}


void its_c1_task_entry(void * arg)
{
	xQueueHandle input_queue = xQueueCreate(10, sizeof(mavlink_message_t));
	assert(input_queue);

	its_c1_rt_endpoint.base.acceptor = accept_message;
	its_c1_rt_endpoint.task_input_queue = input_queue;

	its_rt_sender_ctx_t sctx = {
			.from_isr = 0
	};

	for (;;)
	{
		mavlink_message_t msg;
		BaseType_t rcved = xQueueReceive(input_queue, &msg, 100 / portTICK_PERIOD_MS);
		if (rcved)
			process_message(&msg);

		// Работаем со своей телеметрией
		// FIXME: обеспечить перодичность, даже если
		// нам валом приходят сообщения из очереди
		// и не дают нам поспать наши 100 мс

		uint16_t vcc;
		uint16_t vservo;
		uint16_t flags;
		vcc = vservo = flags = 0;

		// Лезем к дачтикам и выясняем

		// FIXME 42 заменить на какой-нибудь макрос с идентификатором esp32 как системы
		mavlink_msg_power_status_pack(42, ITS_COMP1_MAVCOMP_ID, &msg, vcc, vservo, flags);

		// отправляем пакет в маршрутизацию
		its_rt_route(&sctx, &msg);
	}
}
