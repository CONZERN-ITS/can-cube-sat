/*
 * router.c
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */


#include "router.h"
#include "rt_cfg.h"


#include <stdint.h>

#define MAVLINK_MAX_ID_COUNT 256

#define _RT_XCAT(a, b) a ## b
#define _RT_CAT(a, b) _RT_CAT(a, b)
#define _RT_BASE_NAME

//Creating list
static const int rev_map[] = {
#define _RT_REV_MAP_SET(a, b) [b] = a,
	RT_CFG_LIST(_RT_REV_MAP_SET)
#undef _RT_REV_MAP_SET
};
#define RT_CFG_LIST_SZ sizeof(rev_map) / sizeof(rev_map[0])


//Creating hash
static int its_rt_get_hash(int msgid) {
	switch (msgid) {
#define _RT_SWITCH_SET(a, b) case a: return b;
	RT_CFG_LIST(_RT_SWITCH_SET)
#undef _RT_SWITCH_SET

	default: return RT_CFG_LIST_SZ;
	}
}

typedef struct its_rt_task_identifier list_value;
typedef struct its_rt_task_identifier its_rt_task_identifier;
typedef struct list_node {
	struct list_node *next;
	list_value value;
} list_node;


struct list {
	list_node *first;
};
int list_push(struct list* list, list_value *value) {
	list_node *n = malloc(sizeof(*n));
	if (!n) {
		return -1;
	}
	n->next = list->first;
	n->value = *value;
	list->first = n;
	return 0;
}
void list_delete(struct list* list) {
	list_node *cur = list->first;
	while (cur) {
		list_node *next = cur->next;
		free(cur);
		cur = next;
	}
}

static struct list its_msg_map[RT_CFG_LIST_SZ];


int its_rt_register(int msgid, its_rt_task_identifier task_id) {
	int id = its_rt_get_hash(msgid);
	assert(id != RT_CFG_LIST_SZ);
	return list_push(&its_msg_map[id], &task_id);
}

void its_rt_uninit() {
	for (int i = 0; i < MAVLINK_MAX_ID_COUNT; i++) {
		list_delete(&its_msg_map[i]);
	}
}


void its_rt_route_from_isr(
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg
){

	int id = its_rt_get_hash(msg->msgid);
	if (id != RT_CFG_LIST_SZ) {
		list_node *cur = its_msg_map[id].first;
		BaseType_t higherPrioWoken = 0;
		while (cur) {
			BaseType_t higherPrioWoken2;
			xQueueSendFromISR(cur->value.queue, &msg, &higherPrioWoken2);
			if (higherPrioWoken2) {
				higherPrioWoken = higherPrioWoken2;
			}
		}
		if (higherPrioWoken) {
			portYIELD_FROM_ISR();
		}
		higherPrioWoken = rev_map[0];
	}
}

void its_rt_route(
		const its_rt_sender_ctx_t * sender_ctx,
		const mavlink_message_t * msg,
		TickType_t ticksToWaitForOne
){
	int id = its_rt_get_hash(msg->msgid);
	if (id != RT_CFG_LIST_SZ) {
		list_node *cur = its_msg_map[id].first;
		while (cur) {
			xQueueSend(cur->value.queue, msg, ticksToWaitForOne);
			cur = cur->next;
		}
	}
}
