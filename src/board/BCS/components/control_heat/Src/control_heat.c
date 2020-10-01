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
#include "log_collector.h"
#include "radio.h"

#include <math.h>

static void _task_recv(void *arg);

static void _task_update(void *arg) ;

static int _shift;

typedef enum {
	OFF,
	ON
} state_t;
static state_t state[ITS_BSK_COUNT];
static shift_reg_handler_t *_hsr;
static int consumption[ITS_BSK_COUNT] = {0}; //mA
static int max_consumption = 1;
static float temperature[ITS_BSK_COUNT] = {CONTROL_HEAT_LOWTHD + 1};
int64_t last_time_finite[ITS_BSK_COUNT];

#define FINITE_TIMEOUT 10000 //ms


static TaskHandle_t t_recv;
static TaskHandle_t t_upda;
static log_data_t log_data;



int control_heat_init(shift_reg_handler_t *hsr, int shift, int task_on) {
	for (int i = 0; i < ITS_BSK_COUNT; i++) {
		temperature[i] = CONTROL_HEAT_LOWTHD + 1;
	}
	memset(&log_data, 0, sizeof(log_data));
	_hsr = hsr;
	_shift = shift;

	if (task_on) {
		if (xTaskCreatePinnedToCore(_task_update, "Control heat update", configMINIMAL_STACK_SIZE + 1500, 0, 4, &t_upda, tskNO_AFFINITY) != pdTRUE ||
				xTaskCreatePinnedToCore(_task_recv, "Control heat recv", configMINIMAL_STACK_SIZE + 1500, 0, 3, &t_recv, tskNO_AFFINITY) != pdTRUE) {

			log_data.last_state = LOG_STATE_OFF;
		}
		int64_t now = esp_timer_get_time();
		for (int i = 0; i < ITS_BSK_COUNT; i++) {
			last_time_finite[i] = now;
		}

	}

	if (log_data.last_state == LOG_STATE_OFF) {
		ESP_LOGE("CONTROL_HEAT", "Can't create tasks");
		log_data.error_count++;
		log_data.last_error = LOG_ERROR_LOW_MEMORY;
		log_collector_add(LOG_COMP_ID_SHIFT_REG, &log_data);
	}
	return -1;
}

void control_heat_bsk_enable(int bsk_number, int is_on) {
	shift_reg_set_level_pin(_hsr, _shift + bsk_number * ITS_SR_PACK_SIZE, is_on > 0);
}

void control_heat_set_consumption(int id, int current) {
	consumption[id] = current;
}
void control_heat_set_max_consumption(int current) {
	max_consumption = current;
}


static void _sort(float *arr_sort, int *arr_look, int n) {
	int is_sorted = 0;
	while (!is_sorted) {
		is_sorted = 1;
		for (int i = 0; i < n - 1; i++) {
			if (arr_sort[arr_look[i]] > arr_sort[arr_look[i + 1]]) {
				int t = arr_look[i];
				arr_look[i] = arr_look[i + 1];
				arr_look[i + 1] = t;
				is_sorted = 0;
			}
		}
	}
}


static void _task_recv(void *arg) {
	its_rt_task_identifier tid = {
			.name = "sd_send"
	};
	tid.queue = xQueueCreate(6, MAVLINK_MAX_PACKET_LEN);
	if (!tid.queue) {
		log_data.error_count++;
		log_data.last_error = LOG_ERROR_LOW_MEMORY;
		vTaskDelete(0);
	}
	if (its_rt_register(MAVLINK_MSG_ID_THERMAL_STATE, tid)) {
		ESP_LOGE("CONTROL_HEAT", "not enough memory");
		vQueueDelete(tid.queue);
		log_data.error_count++;
		log_data.last_error = LOG_ERROR_LOW_MEMORY;
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
		ESP_LOGD("CONTROL_HEAT", "TEMP: %d %f", msg.compid, mts.temperature);
		if (isfinite(mts.temperature)) {
			temperature[msg.compid] = mts.temperature;
			last_time_finite[msg.compid] = esp_timer_get_time();
		}
	}
}
#define __CAT(a,b) a ## b
#define __BSK(i) __CAT(ITS_BSK, i)

static void _task_update(void *arg) {
	int64_t now = esp_timer_get_time();
	//Массив для сортировки не сортируя
	int arr[ITS_BSK_COUNT] = {0};


	while (1) {
		for (int i = 0; i < ITS_BSK_COUNT; i++) {
			arr[i] = i;
		}
		//Поуправляем радио. Если плата слишком горячая, то...
		if (temperature[0] > CONTROL_HEAT_RADIO_HIGH_THD) {
			portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
			taskENTER_CRITICAL(&myMutex);
			radio_send_set_baud_koef(0.10);
			taskEXIT_CRITICAL(&myMutex);
			ESP_LOGD("CONTROL_HEAT", "TOO hot! Radio OFF");

		}
		if (temperature[0] < CONTROL_HEAT_RADIO_LOW_THD) {
			portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
			taskENTER_CRITICAL(&myMutex);
			radio_send_set_baud_koef(1);
			taskEXIT_CRITICAL(&myMutex);
			ESP_LOGD("CONTROL_HEAT", "Cool! Radio ON");
		}
		//Выключим те, от которых ничего не слышно
		for (int i = 0; i < ITS_BSK_COUNT; i++) {
			if ((now - last_time_finite[i]) / 1000 > FINITE_TIMEOUT) {
				state[i] = OFF;
				//Подправим температуру, чтобы оно не включилось из-за низкой температуры
				temperature[i] = CONTROL_HEAT_LOWTHD + 1;
				ESP_LOGD("CONTROL_HEAT", "No response from %d sensor. Switching off", i);
			}
		}
		//Отсортируем. Теперь temperature[arr[i]] отсортированно по i по возрастанию
		_sort(temperature, arr, ITS_BSK_COUNT);
		//Подготовим новые состояния
		state_t new[ITS_BSK_COUNT] = {0};
		//Масимальный ток потребления
		int total_consumption = 0;
		for (int i = 0; i < ITS_BSK_COUNT; i++) {
			ESP_LOGD("CONTROL_HEAT", "TEMP: %d %d %f", i, arr[i], temperature[arr[i]]);
			//Если температура низкая и достаточно тока в запасе - вклчючаем.
			//Самые холодные будут точно включены, так как они первые проходили эту проверку
			if (temperature[arr[i]] < CONTROL_HEAT_LOWTHD &&
					(total_consumption + consumption[arr[i]] < max_consumption)) {
				new[arr[i]] = ON;
				total_consumption += consumption[arr[i]];
			} else {
				new[arr[i]] = OFF;
			}
		}
		//Запишем на сдвиговые регистры новое состояние
		for (int sensor = 0; sensor < ITS_BSK_COUNT; sensor++) {
			//Между порядком датчиков и порядком нагревателей не прямое соответствие
			int heater = 0;
			switch (sensor) {
			case 0: heater = ITS_BSK_1; break;
			case 1: heater = ITS_BSK_2; break;
			case 2: heater = ITS_BSK_3; break;
			case 3: heater = ITS_BSK_4; break;
			case 4: heater = ITS_BSK_5; break;
			case 5: heater = ITS_BSK_2A; break;
			default: heater = -1;
			}
			if (heater >= 0) {
				shift_reg_set_level_pin(_hsr, _shift + ITS_SR_PACK_SIZE * heater, new[sensor] == ON);
			}
		}
		esp_err_t rc = shift_reg_load(_hsr);
		if (rc == ESP_OK) {
			for (int i = 0; i < ITS_BSK_COUNT; i++) {
				if (new[i] == ON && state[i] == OFF) {
					ESP_LOGD("CONTROL_HEAT", "now ON %d %f", i, temperature[i]);
				}
				if (new[i] == OFF && state[i] == ON) {
					ESP_LOGD("CONTROL_HEAT", "now OFF %d %f", i, temperature[i]);
				}
				state[i] = new[i];
			}
		} else {
			ESP_LOGE("CONTROL_HEAT", "bad spi %d", rc);
			log_data.last_error = LOG_ERROR_LL_API;
			log_data.error_count++;
		}
		//Соберем немного логов
		log_collector_add(LOG_COMP_ID_SHIFT_REG, &log_data);
		vTaskDelay(CONTROL_HEAT_UPDATE_PERIOD / portTICK_PERIOD_MS);
	}

}

/*
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
}*/
