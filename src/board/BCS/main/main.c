/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "errno.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task.h"
#include "esp_sntp.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "ark_time_sync.h"
#include "init_helper.h"
#include "imi.h"
#include "router.h"
#include "internet.h"
#include "mavlink_help2.h"
#include "radio.h"




static void task_print_telemetry(void *pvParameters);
static void task_send_telemetry_wifi(void *pvParameters);
static void task_recv_telemetry_wifi(void *pvParameters);
static void task_send_telemetry_uart(void *pvParameters);


void app_main(void)
{
	init_helper();

	//xTaskCreatePinnedToCore(task_socket_comm, "Socket communication", configMINIMAL_STACK_SIZE + 4000, "Socket comm", 1, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(task_print_telemetry, "Print telemetry", configMINIMAL_STACK_SIZE + 4000, "Print telemetry", 1, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(ark_tsync_task, "ARK time sync", configMINIMAL_STACK_SIZE + 4000, "ARK time sync", 1, 0, tskNO_AFFINITY);
#if ITS_WIFI_SERVER
	//xTaskCreatePinnedToCore(task_send_telemetry_uart, "Send tel", configMINIMAL_STACK_SIZE + 2048, 0, 2, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(task_recv_telemetry_wifi, "Send tel", configMINIMAL_STACK_SIZE + 5000, 0, 2, 0, tskNO_AFFINITY);
#else
	xTaskCreatePinnedToCore(task_send_telemetry_wifi, "Send tel", configMINIMAL_STACK_SIZE + 4000, 0, 2, 0, tskNO_AFFINITY);
#endif

	//xTaskCreatePinnedToCore(task_socket_recv, "Recv", configMINIMAL_STACK_SIZE + 4000, 0, 4, 0, tskNO_AFFINITY);



	ESP_LOGI("SYSTEM", "Tasks created");

}

static void task_send_telemetry_wifi(void *pvParameters) {

	its_rt_task_identifier tid;
	tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN);
	its_rt_register_for_all(tid);


	struct sockaddr_in addr = {0};

	addr.sin_addr.s_addr = ITS_WIFI_SERVER_ADDRESS.u_addr.ip4.addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ITS_WIFI_PORT);

	int sout;
	sout = socket(AF_INET, SOCK_DGRAM, 0);
	while (1) {

		mavlink_message_t msg;

		if (!xQueueReceive(tid.queue, &msg, portMAX_DELAY)) {
			ESP_LOGE("INET", "Can't read from queue");
			vTaskDelay(500 / portTICK_RATE_MS);
			continue;
		}
		ESP_LOGV("INET", "Got smthng to send");
		uint8_t buf[MAVLINK_MAX_PACKET_LEN];
		int count = mavlink_msg_to_send_buffer(buf, &msg);

		sendto(sout, (uint8_t *) buf, count, 0, (struct sockaddr *)&addr, sizeof(addr));
		//TODO: надо бы запоминать, когда не отправилось
	}
	vTaskDelete(NULL);
}

static void task_recv_telemetry_wifi(void *pvParameters) {
	int sin;
	sin = socket(AF_INET, SOCK_DGRAM, 0);
	if (sin < 0) {
		ESP_LOGE("INET", "Can't create socket");
	}
	struct sockaddr_in addr = {0};
	addr.sin_addr.s_addr = ITS_WIFI_SERVER_ADDRESS.u_addr.ip4.addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ITS_WIFI_PORT);
	if (bind(sin, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		ESP_LOGE("INET", "Can't bind socket");
	}

	int chan = mavlink_claim_channel();
	mavlink_message_t msg;
	mavlink_status_t mst;
	while (1) {
		uint8_t buf[MAVLINK_MAX_PACKET_LEN];
		socklen_t len = sizeof(addr);
		int size = recvfrom(sin, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		for (int i = 0; i < size; ++i) {
			//printf("%02X", buf[i]);

			if(mavlink_parse_char(chan, buf[i], &msg, &mst)) {
				its_rt_sender_ctx_t ctx = {0};
				ctx.from_isr = 0;
				its_rt_route(&ctx, &msg, 0);

			}
		}
		//TODO: надо бы запоминать, когда не отправилось
	}
	vTaskDelete(NULL);
}
#define PERIOD 700 //ms

typedef struct  {
	uint32_t baud_send;
	uint32_t buffer_size;
	uart_port_t port;
	uint32_t low_thrld;
	uint32_t high_thrld;
} safe_send_cfg_t;

typedef struct  {
	safe_send_cfg_t cfg;
	int32_t filled;
	int64_t last_checked;
} safe_send_t;
void safe_uart_send(safe_send_t *h, uint8_t *buf, uint16_t size) {
	uint32_t Bs = h->cfg.baud_send / 8;

	int64_t now = esp_timer_get_time();
	h->filled -= ((now - h->last_checked) * Bs) / 1000000;
	h->filled = h->filled > 0 ? h->filled : 0;


	while (size > 0) {
		if (h->filled >= h->cfg.high_thrld) {
			uint32_t ttt = Bs * portTICK_PERIOD_MS;
			uint32_t ticks = ((h->filled - h->cfg.low_thrld) * 1000 + ttt - 1) / ttt;
			vTaskDelay(ticks);
			h->filled -= ticks * portTICK_PERIOD_MS * Bs / 1000;
		} else {
			uint32_t maxSend = (h->cfg.buffer_size - h->filled);

			uint16_t s = size;
			if (size >= maxSend) {
				s = maxSend;
			}
			int64_t start = esp_timer_get_time();
			uart_write_bytes(h->cfg.port, (char *) buf, s);
			int64_t now = esp_timer_get_time();

			h->filled -= (Bs * (now - start)) / 1000000;
			h->filled += s;
			h->filled = h->filled > 0 ? h->filled : 0;

			buf += s;
			size -= s;

		}
	}
	h->last_checked = esp_timer_get_time();

}

static void task_send_telemetry_uart(void *pvParameters) {
	its_rt_task_identifier tid;
	tid.queue = xQueueCreate(40, MAVLINK_MAX_PACKET_LEN);
	its_rt_register_for_all(tid);
	safe_send_t sst = {0};
	sst.cfg.low_thrld = 0;
	sst.cfg.high_thrld = 1000;
	sst.cfg.baud_send = 720;
	sst.cfg.buffer_size = 1000;
	sst.cfg.port = ITS_UART0_PORT;

	while (1) {
		mavlink_message_t msg;

		if (xQueueReceive(tid.queue, &msg, portMAX_DELAY) == pdFALSE) {
			ESP_LOGE("UART_RADIO", "BAD");
			vTaskDelay(500 / portTICK_RATE_MS);
			continue;
		}
		//ESP_LOGI("UART_RADIO", "Got smthng to send");
		uint8_t buf[MAVLINK_MAX_PACKET_LEN];
		int count = mavlink_msg_to_send_buffer(buf, &msg);
		safe_uart_send(&sst, buf, count);
		//uart_write_bytes(ITS_UART0_PORT, (char *) buf, count);
		//vTaskDelay(PERIOD / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

static void task_print_telemetry(void *pvParameters) {
	while (1) {
		struct timeval tm;
		gettimeofday(&tm, 0);
		printf("TIME: %d.%06d\n", (uint32_t)tm.tv_sec, (uint32_t)tm.tv_usec);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	its_rt_task_identifier tid;
	tid.queue = xQueueCreate(5, MAVLINK_MAX_PACKET_LEN);
	printf("HH: %d\n", (int)tid.queue);
	its_rt_register(MAVLINK_MSG_ID_TIMESTAMP, tid);
	TickType_t prev1 = xTaskGetTickCount();
	while (1) {
		mavlink_message_t msg;
		if (xQueueReceive(tid.queue, (uint8_t*) &msg, 0)) {
			switch (msg.msgid) {
			case MAVLINK_MSG_ID_TIMESTAMP:
			{
				mavlink_timestamp_t mts;
				mavlink_msg_timestamp_decode(&msg, &mts);
				printf("STM TIME: \t%u.%03u\n", (uint32_t) mts.time_s, mts.time_us / 1000);
			}
			}
		}
		if (xTaskGetTickCount() - prev1 > 1000 / portTICK_PERIOD_MS) {
			prev1 += 1000 / portTICK_PERIOD_MS;
			char buf[100];
			time_t t = time(0);
			struct tm* tm= gmtime(&t);

			strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", tm);

			printf("TIME : \t\t%s\n", buf);
			struct timeval tp;
			gettimeofday(&tp, 0);
			printf("TIME2: \t\t%lu.%ld\n", tp.tv_sec, tp.tv_usec);
		}
		vTaskDelay(100 / portTICK_RATE_MS);

	}
}



