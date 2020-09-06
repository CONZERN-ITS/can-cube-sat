/*
 * log_collector.c
 *
 *  Created on: Aug 19, 2020
 *      Author: sereshotes
 */

#include "log_collector.h"

#include "mavlink_help2.h"
#include "router.h"
#include "init_helper.h"


static log_collector_t _coll;

static void log_collector_task(void *arg) {
	log_collector_t *coll = (log_collector_t *)arg;
	TickType_t xLastWakeTime;

	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		mavlink_bcu_stats_t mbs = {0};
		mavlink_message_t msg = {0};
		struct timeval tv = {0};
		gettimeofday(0, &tv);
		mbs.time_s = tv.tv_sec;
		mbs.time_us = tv.tv_usec;

		mbs.sd_elapsed_time_from_msg 	= coll->log_data[LOG_COMP_ID_SD].ellapsed_time;
		mbs.sd_error_count 				= coll->log_data[LOG_COMP_ID_SD].error_count;
		mbs.sd_last_error 				= coll->log_data[LOG_COMP_ID_SD].last_error;
		mbs.sd_last_state 				= coll->log_data[LOG_COMP_ID_SD].last_state;

		mavlink_msg_bcu_stats_encode(mavlink_system, COMP_ANY_0, &msg, &mbs);
		its_rt_sender_ctx_t ctx = {0};
		ctx.from_isr = 0;
		its_rt_route(&ctx, &msg, 100 / portTICK_PERIOD_MS);
		vTaskDelayUntil(&xLastWakeTime, LOG_COLLECTOR_SEND_PERIOD / portTICK_PERIOD_MS);
	}
}

void log_collector_init(log_collector_t * coll) {
	if (!coll) {
		coll = &_coll;
	}
	memset(coll, 0, sizeof(*coll));
	xTaskCreate(log_collector_task, "Log collector", configMINIMAL_STACK_SIZE + 2500, coll, 2, 0);
}

void log_collector_add_to(log_collector_t *hlc, log_comp_id_t id, const log_data_t *data) {
	hlc->log_data[id] = *data;
}

void log_collector_add(log_comp_id_t id, const log_data_t *data) {
	_coll.log_data[id] = *data;
}
void log_collector_log_task(log_data_t *data) {
	while (1) {
		if (data && data->last_state == LOG_STATE_OFF) {
			vTaskDelete(0);
		}
		log_collector_add(LOG_COMP_ID_SHIFT_REG, &data);
		vTaskDelay(LOG_COLLECTOR_ADD_PERIOD_COMMON / portTICK_PERIOD_MS);
	}

}
