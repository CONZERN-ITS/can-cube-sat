/*
 * router.c
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */
/*
 * Данный модуль релизует центраьный хаб всех данных.
 *
 * Все данные в системе представленны в виде mavlink сообщений,
 * причем каждый имеет собственный идентификатор, позволяющий
 * различать их между собой.
 *
 * Каждый таск (и не только) может зарегестрировать свою очередь
 * в этом модуле для необходимых ему типов сообщений. Так как все
 * сообщения, получаемые МК, попадают в этот модуль, то данный таск
 * получить все возможные сообщения с заданным msgid.
 *
 * Очереди сохраняются в виде списков очередей для каждого указанного
 * в RT_CFG_LIST типа сообщения.
 *
 * Регистрировать очередь можно с помощью функций:
 * its_rt_register - для одного типа
 * its_rt_register_for_all - для вообще всех типов (и не только указанных)
 *
 * Отправлять сообщения в этот модуль можно через:
 * its_rt_route - для вызова не из прерываний
 * its_rt_route_from_isr - при вызове из прерываний
 */

#include "router.h"
#include "rt_cfg.h"
#include "esp_log.h"


#include <stdint.h>

#define MAVLINK_MAX_ID_COUNT 256


/*
 * Массив идентификаторов сообщений
 */
static const int rev_map[] = {
#define _RT_REV_MAP_SET(a, b) [b] = a,
	RT_CFG_LIST(_RT_REV_MAP_SET)
#undef _RT_REV_MAP_SET
};
#define RT_CFG_LIST_SZ sizeof(rev_map) / sizeof(rev_map[0])


/*
 * Хэш-функция для получения индекса в массиве rev_map для заданного
 * идентификатора сообщения.
 */
static int its_rt_get_hash(int msgid) {
	switch (msgid) {
#define _RT_SWITCH_SET(a, b) case a: return b;
	RT_CFG_LIST(_RT_SWITCH_SET)
#undef _RT_SWITCH_SET

	default: return RT_CFG_LIST_SZ;
	}
}
////////////////////////////////////////////////////
/*
 * Список, и его базовые функции
 */
typedef struct its_rt_task_identifier list_value;
typedef struct its_rt_task_identifier its_rt_task_identifier;
typedef struct list_node {
	struct list_node *next;
	list_value value;
} list_node;


struct list {
	list_node *first;
};
static int list_push(struct list* list, list_value *value) {
	list_node *n = malloc(sizeof(*n));
	if (!n) {
		return -1;
	}
	n->next = list->first;
	n->value = *value;
	list->first = n;
	return 0;
}
static void list_delete(struct list* list) {
	list_node *cur = list->first;
	while (cur) {
		list_node *next = cur->next;
		free(cur);
		cur = next;
	}
}
////////////////////////////////////////////

/*
 * Массив списков очередей
 */
static struct list its_msg_map[RT_CFG_LIST_SZ];
static struct list its_msg_all;

/*
 * Регистрация заданной очереди для данного идентификатора
 */
int its_rt_register(int msgid, its_rt_task_identifier task_id) {
	int id = its_rt_get_hash(msgid);
	assert(id != RT_CFG_LIST_SZ);
	return list_push(&its_msg_map[id], &task_id);
}
/*
 * Регистрация заданной очереди для всех типов сообщений
 */
int its_rt_register_for_all(its_rt_task_identifier task_id) {
	return list_push(&its_msg_all, &task_id);
}

/*
 * Удаление модуля
 */
void its_rt_uninit() {
	for (int i = 0; i < MAVLINK_MAX_ID_COUNT; i++) {
		list_delete(&its_msg_map[i]);
	}
	list_delete(&its_msg_all);
}

static BaseType_t _route_from_isr(list_node *cur,
		const mavlink_message_t * msg) {

	BaseType_t higherPrioWoken = 0;
	while (cur) {
		BaseType_t higherPrioWoken2;
		xQueueSendFromISR(cur->value.queue, &msg, &higherPrioWoken2);
		if (higherPrioWoken2) {
			higherPrioWoken = higherPrioWoken2;
		}
	}
	return higherPrioWoken;
}

void its_rt_route_from_isr(
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg
){

	int id = its_rt_get_hash(msg->msgid);

	BaseType_t higherPrioWoken = 0;
	if (id != RT_CFG_LIST_SZ) {
		higherPrioWoken |= _route_from_isr(its_msg_map[id].first, msg);
	}
	higherPrioWoken |= _route_from_isr(its_msg_all.first, msg);
	if (higherPrioWoken) {
		portYIELD_FROM_ISR();
	}
	higherPrioWoken = rev_map[0];
}

static void _route(list_node *cur,
		const mavlink_message_t * msg,
		TickType_t ticksToWaitForOne) {
	while (cur) {
		xQueueSend(cur->value.queue, msg, ticksToWaitForOne);
		cur = cur->next;
	}
}
/*
 * Отправка сообщения всем очередям соответствующего типа
 */
void its_rt_route(
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg,
		TickType_t ticksToWaitForOne
){
	ESP_LOGI("ROUTER", "Got message %d from %d:%d", msg->msgid, msg->sysid, msg->compid);
	int id = its_rt_get_hash(msg->msgid);
	if (id != RT_CFG_LIST_SZ) {
		_route(its_msg_map[id].first, msg, ticksToWaitForOne);
	}
	_route(its_msg_all.first, msg, ticksToWaitForOne);
}
