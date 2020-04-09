/*
 * task.c
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#include "comp1.h"

#include "../router/router.h"

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

	//its_c1_rt_endpoint.base.acceptor = accept_message;
	its_c1_rt_endpoint.task_input_queue = input_queue;

	its_rt_sender_ctx_t sctx = {
			.from_isr = 0
	};
	const int queueSize = 10;
	QueueHandle_t qh = xQueueCreate(queueSize, MAVLINK_MAX_PACKET_LEN);
	its_rt_task_identifier tid = {
			.queue = qh
	};
	its_rt_register(MAVLINK_MSG_ID_GPS_UBX_NAV_SOL, tid);

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
		mavlink_msg_power_status_pack(42, 0, &msg, vcc, vservo, flags);

		// отправляем пакет в маршрутизацию
		its_rt_route(&sctx, &msg, portMAX_DELAY);
	}
}
