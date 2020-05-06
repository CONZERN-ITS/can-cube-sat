/*
 * ark_time_sync.c
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#include "ark_time_sync.h"

#include "mavlink/its/mavlink.h"
#include "driver/gpio.h"

#include <sys/time.h>
#include "init_helper.h"
#include "pinout_cfg.h"
#include "imi.h"


int ark_tsync_send_signal(uint8_t *data, size_t *size) {
	struct timeval tp;

	TickType_t start = xTaskGetTickCount();
	gpio_set_level(ITS_PIN_TIME, 0);
	gettimeofday(&tp, 0);

	mavlink_timestamp_t mts;
	mavlink_message_t msg;

	mts.time_s = tp.tv_sec;
	mts.time_us = tp.tv_usec;
	mts.time_base = 0;

	const int magic_number = 42; //FIXME: Надо добавить системы
	mavlink_msg_timestamp_encode(magic_number, magic_number, &msg, &mts);
	assert (*size >= mavlink_max_message_length(&msg));
	*size = mavlink_msg_to_send_buffer(data, &msg);
	vTaskDelayUntil(&start, ARK_SIGNAL_LENGTH / portTICK_RATE_MS);

	gpio_set_level(ITS_PIN_TIME, 1);
	return 0;
}


void ark_tsync_task(void *pvParametres) {
	while (1) {
		uint8_t packet[MAVLINK_MAX_PACKET_LEN];
		size_t size = sizeof(packet);
		ark_tsync_send_signal(packet, &size);

		imi_send_all(0, packet, size, 100 / portTICK_RATE_MS);
		vTaskDelay(ARK_TIME_SYNC_PRIOD / portTICK_RATE_MS);
	}
}
