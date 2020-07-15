/*
 * time_sync.c
 *
 *  Created on: Jul 14, 2020
 *      Author: sereshotes
 */
#include "time_sync.h"
#include "router.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include <sys/time.h>

#include "freertos/queue.h"
#include "pinout_cfg.h"

#include "mavlink/its/mavlink.h"


//static void adjust(struct timeval *t1, struct timeval *t2)
#define TIME_SYNC_SMOOTH_THREASHOLD 60 //секунд
static void time_sync_task(void *arg) {
	ts_sync *ts = (ts_sync *)arg;

	its_rt_task_identifier id = {0};
	id.queue = xQueueCreate(3, MAVLINK_MAX_PACKET_LEN);

	its_rt_register(MAVLINK_MSG_ID_TIMESTAMP, id);
	while (1) {
		mavlink_message_t msg;
		if (!xQueueReceive(id.queue, &msg, portMAX_DELAY)) {
			vTaskDelay(5000 / portTICK_RATE_MS);
			continue;
		}

		if (msg.compid != CUBE_1_SINS) {
			ESP_LOGI("TIME SYNC","Who is sending it?");
			continue;
		}
		if (!ts->is_updated) {
			ESP_LOGI("TIME SYNC","Where is signal?");
			continue;
		}

		struct timeval there;
		mavlink_timestamp_t mts;
		mavlink_msg_timestamp_decode(&msg, &mts);
		there.tv_sec = mts.time_s;
		there.tv_usec = mts.time_us;
		const struct timeval delta = {
				.tv_sec = there.tv_sec - ts->here.tv_sec,
				.tv_usec = there.tv_usec - ts->here.tv_usec
		};


		if (abs(delta.tv_sec) > TIME_SYNC_SMOOTH_THREASHOLD) {
			ESP_LOGI("TIME SYNC","Too big diff. Immidiate sync!");
			struct timeval t;
			gettimeofday(&t, 0);
			t.tv_sec += delta.tv_sec;
			t.tv_usec += delta.tv_usec;
			settimeofday(&t, 0);
			continue;
		} else {
			adjtime(&delta, 0);
		}
	}

}

static void IRAM_ATTR isr_handler(void *arg) {
	ts_sync *ts = (ts_sync *)arg;
	gettimeofday(&ts->here, 0);
	ts->is_updated = 1;
}

void time_sync_sins_install(ts_sync *cfg) {

	xTaskCreate(time_sync_task, "timesync", 4096, cfg, 1, NULL);
	gpio_isr_handler_add(cfg->pin, isr_handler, cfg);
}
void time_sync_bcs_install() {

}

