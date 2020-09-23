/*
 * its_uart.c
 *
 *  Created on: 23 апр. 2020 г.
 *      Author: sereshotes
 */
#include "uart_mavlink.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "mavlink/its/mavlink.h"
#include "mavlink_help2.h"
#include "router.h"
#include "log_collector.h"

static const char *TAG = "UART_MAVLINK";

#define TASK_BUF_SIZE 10
#define UART_MAV_CHAN MAVLINK_COMM_1
#define UART_EV_BUF_SIZE 20

static log_data_t log_data;
static int64_t last_time;

struct _install_set {
	QueueHandle_t queue;
	mavlink_channel_t channel;
	uart_port_t uart;
};

static void _parse_buf(mavlink_channel_t chan, uint8_t *buffer, uint16_t size) {
	mavlink_message_t msg;
	mavlink_status_t mst;
	for (int i = 0; i < size; i++) {
		if (mavlink_parse_char(chan, buffer[i], &msg, &mst)) {
			its_rt_sender_ctx_t ctx = {0};
			ctx.from_isr = 0;
			its_rt_route(&ctx, &msg, 20 / portTICK_RATE_MS);
		}
	}
}

void uart_event_task(void *pvParameters)
{
	struct _install_set *is = (struct _install_set *)pvParameters;
	QueueHandle_t uart_queue = is->queue;
	mavlink_channel_t chan = is->channel;
	uart_port_t uart = is->uart;
	free(pvParameters);

	uart_event_t event;
	uint8_t buffer[TASK_BUF_SIZE];
	while (1) {
		if (log_data.last_state == LOG_STATE_OFF) {
			vTaskDelete(0);
		}
		//Waiting for UART event.
		if(xQueueReceive(uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
			last_time = esp_timer_get_time();

			fflush(stdout);
			bzero(buffer, TASK_BUF_SIZE);
			//ESP_LOGI(TAG, "uart[%d] event:", uart);
			log_data.last_error = event.type;
			if (event.type != UART_DATA) {
				log_data.error_count++;
			}
			switch(event.type) {
			//Event of UART receving data
			/*We'd better handler data event fast, there would be much more data events than
			other types of events. If we take too much time on data event, the queue might
			be full.*/
			case UART_FIFO_OVF: {
				ESP_LOGE(TAG, "hw fifo overflow");
			}
				/* no break */
			case UART_BUFFER_FULL: {
				ESP_LOGI(TAG, "ring buffer full");
			}
				/* no break */
			case UART_DATA:{
				//ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
				uint16_t remain = event.size;

				do {
					uint16_t try_read = remain < TASK_BUF_SIZE ? remain : TASK_BUF_SIZE;
					int16_t count = 0;
					count = uart_read_bytes(uart, buffer, try_read, portMAX_DELAY);
					if (count < 0) {
						ESP_LOGE(TAG, "ERROR: uart error");
						continue;
					}
					_parse_buf(chan, buffer, (uint16_t) count);
					remain -= count;
				} while (remain);

				break;
			}
			case UART_BREAK:
				ESP_LOGI(TAG, "uart rx break");
				break;
				//Event of UART parity check error
			case UART_PARITY_ERR:
				ESP_LOGI(TAG, "uart parity error");
				break;
			//Event of UART frame error
			case UART_FRAME_ERR:
				ESP_LOGI(TAG, "uart frame error");
				break;
			//Others
			default:
				ESP_LOGI(TAG, "uart event type: %d", event.type);
				break;
			}
		}
	}
	vTaskDelete(NULL);
}
static void _log(log_data_t *data) {
	while (1) {
		if (data && data->last_state == LOG_STATE_OFF) {
			vTaskDelete(0);
		}
		log_data.ellapsed_time = (esp_timer_get_time() - last_time) / 1000;
		log_collector_add(LOG_COMP_ID_SINC_COMM, data);
		vTaskDelay(LOG_COLLECTOR_ADD_PERIOD_COMMON / portTICK_PERIOD_MS);
	}
}
int uart_mavlink_install(uart_port_t uart_num, QueueHandle_t uart_queue) {
	mavlink_channel_t chan = mavlink_claim_channel();

	struct _install_set *t = malloc(sizeof(*t));
	t->queue = uart_queue;
	t->channel = chan;
	t->uart = uart_num;
	log_data.last_state = LOG_STATE_ON;
	last_time = esp_timer_get_time() / 1000;
	//Create a task to handler UART event from ISR
	if (xTaskCreate(uart_event_task, "uart_mavlink", 4096, t, 1, NULL) != pdTRUE ||
			xTaskCreate(_log, "uart_mavlink_log", 1500, &log_data, 2, NULL) != pdTRUE) {
		log_data.last_state = LOG_STATE_OFF;
		free(t);
		ESP_LOGE(TAG, "Can't create task");
		return -1;
	}
	return 0;
}
int uart_mavlink_uninstall(uart_port_t uart_num, QueueHandle_t uart_queue) {
	log_data.last_state = LOG_STATE_OFF;
	return 0;
}
