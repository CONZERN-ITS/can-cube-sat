/*
 * control_heat.c
 *
 *  Created on: Aug 8, 2020
 *      Author: sereshotes
 */

#include "control_heat.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "router.h"
#include "mavlink_help2.h"
#include "init_helper.h"


static void control_heat_task(void *arg);

static int _shift;

typedef enum {
	OFF,
	ON
} state_t;
static state_t state[ITS_BSK_COUNT];
static shift_reg_handler_t *_hsr;

void control_heat_init(shift_reg_handler_t *hsr, int shift, int task_on) {
	xTaskCreatePinnedToCore(control_heat_task, "Control heat task", configMINIMAL_STACK_SIZE + 1000, 0, 3, 0, tskNO_AFFINITY);
	_shift = shift;
	_hsr = hsr;
}

void control_heat_bsk_enable(int bsk_number, int is_on) {
	shift_reg_set_level_pin(_hsr, _shift + bsk_number * ITS_SR_PACK_SIZE, is_on > 0);
}


static void control_heat_task(void *arg) {

	its_rt_task_identifier tid = {
			.name = "sd_send"
	};
	tid.queue = xQueueCreate(6, MAVLINK_MAX_PACKET_LEN);
	if (!tid.queue || its_rt_register(MAVLINK_MSG_ID_THERMAL_STATE, tid)) {
		ESP_LOGE("CONTROL_HEAT", "not enough memory");
		vTaskDelete(0);
	}

	while (1) {
		mavlink_message_t msg = {0};
		xQueueReceive(tid.queue, &msg, portMAX_DELAY);
		if (msg.sysid != mavlink_system) {
			continue;
		}
		mavlink_thermal_state_t mts = {0};
		mavlink_msg_thermal_state_decode(&msg, &mts);
		if (mts.temperature > CONTROL_HEAT_HIGHTHD && state[msg.compid] == ON) {
			shift_reg_set_level_pin(_hsr, _shift + ITS_SR_PACK_SIZE * msg.compid, 0);
			esp_err_t rc = shift_reg_load(_hsr);
			if (rc == ESP_OK) {
				state[msg.compid] = OFF;
				ESP_LOGD("CONTROL_HEAT", "themp switched off for %d", msg.compid);
			} else {
				ESP_LOGE("CONTROL_HEAT", "bad spi %d", rc);
			}
		}
		if (mts.temperature < CONTROL_HEAT_LOWTHD && state[msg.compid] == OFF) {
			shift_reg_set_level_pin(_hsr, _shift + ITS_SR_PACK_SIZE * msg.compid, 1);
			esp_err_t rc = shift_reg_load(_hsr);
			if (rc == ESP_OK) {
				state[msg.compid] = ON;
				ESP_LOGD("CONTROL_HEAT", "themp switched on for %d", msg.compid);
			} else {
				ESP_LOGE("CONTROL_HEAT", "bad spi %d", rc);
			}
		}
	}
}
