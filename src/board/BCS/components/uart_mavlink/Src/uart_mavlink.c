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

static const char *TAG = "uart_mavlink_events";

/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

#define EX_UART_NUM UART_NUM_0

#define TASK_BUF_SIZE 10
#define UART_MAV_CHAN MAVLINK_COMM_1
#define UART_EV_BUF_SIZE 20


struct _install_set {
	QueueHandle_t queue;
	mavlink_channel_t channel;
};

static void _parse_buf(mavlink_channel_t chan, uint8_t *buffer, uint16_t size) {
	mavlink_message_t msg;
	mavlink_status_t mst;
	for (int i = 0; i < size; i++) {
		if (mavlink_parse_char(chan, buffer[i], &msg, &mst)) {
			its_rt_sender_ctx_t ctx = {0};
			ctx.from_isr = 0;
			its_rt_route(&ctx, &msg, portMAX_DELAY);
		}
	}
}

void uart_event_task(void *pvParameters)
{
	struct _install_set *is = (struct _install_set *)pvParameters;
	QueueHandle_t uart_queue = is->queue;
	mavlink_channel_t chan = is->channel;
	free(pvParameters);

	uart_event_t event;
	uint8_t buffer[TASK_BUF_SIZE];
	while (1) {
		//Waiting for UART event.
		if(xQueueReceive(uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
			fflush(stdout);
			bzero(buffer, TASK_BUF_SIZE);
			ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
			switch(event.type) {
			//Event of UART receving data
			/*We'd better handler data event fast, there would be much more data events than
			other types of events. If we take too much time on data event, the queue might
			be full.*/
			case UART_DATA:
				ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
				uint16_t remain = event.size;

				do {
					uint16_t try_read = remain < TASK_BUF_SIZE ? remain : TASK_BUF_SIZE;
					int16_t count = uart_read_bytes(EX_UART_NUM, buffer, try_read, portMAX_DELAY);
					if (count < 0) {
						ESP_LOGE(TAG, "ERROR: uart error");
						continue;
					}
					_parse_buf(chan, buffer, (uint16_t) count);
					remain -= count;
				} while (remain);

				break;
			//Event of HW FIFO overflow detected
			case UART_FIFO_OVF:
				ESP_LOGI(TAG, "hw fifo overflow");
				break;
			//Event of UART ring buffer full
			case UART_BUFFER_FULL:
				ESP_LOGI(TAG, "ring buffer full");
				break;
			//Event of UART RX break detected
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

void uart_mavlink_install(uart_port_t uart_num, QueueHandle_t uart_queue) {

	esp_log_level_set(TAG, ESP_LOG_INFO);
	mavlink_channel_t chan = mavlink_claim_channel();

	struct _install_set *t = malloc(sizeof(t));
	t->queue = uart_queue;
	t->channel = chan;
	//Create a task to handler UART event from ISR
	xTaskCreate(uart_event_task, "uart_event_task", 2048, t, 1, NULL);
}
