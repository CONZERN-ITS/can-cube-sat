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



void app_main(void)
{
	init_helper();

	//xTaskCreatePinnedToCore(task_socket_comm, "Socket communication", configMINIMAL_STACK_SIZE + 4000, "Socket comm", 1, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(task_print_telemetry, "Print telemetry", configMINIMAL_STACK_SIZE + 4000, "Print telemetry", 1, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(ark_tsync_task, "ARK time sync", configMINIMAL_STACK_SIZE + 4000, "ARK time sync", 1, 0, tskNO_AFFINITY);
#if ITS_WIFI_SERVER
	xTaskCreatePinnedToCore(task_recv_telemetry_wifi, "Send tel", configMINIMAL_STACK_SIZE + 5000, 0, 2, 0, tskNO_AFFINITY);
#else
	xTaskCreatePinnedToCore(task_send_telemetry_wifi, "Send tel", configMINIMAL_STACK_SIZE + 4000, 0, 2, 0, tskNO_AFFINITY);
#endif

	ESP_LOGI("SYSTEM", "Tasks created");

}

static void task_send_telemetry_wifi(void *pvParameters) {

	its_rt_task_identifier tid = {
			.name = "wifi_send"
	};
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

static void task_print_telemetry(void *pvParameters) {
	while (1) {
		struct timeval tm;
		gettimeofday(&tm, 0);
		printf("TIME: %d.%06d\n", (uint32_t)tm.tv_sec, (uint32_t)tm.tv_usec);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	its_rt_task_identifier tid = {
			.name = "print_tel"
	};
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



