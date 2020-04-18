/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <internet.h>
#include <string.h>
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


static const char *TAG = "wifi station";





static void task_send_telemetry(void *pvParameters);


void app_main(void)
{
    init_helper();
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    printf("Wifi inited\n");

    xTaskCreatePinnedToCore(task_socket_comm, "Socket communication", configMINIMAL_STACK_SIZE + 4000, "Socket comm", 1, 0, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(task_send_telemetry, "Send telemetry", configMINIMAL_STACK_SIZE + 4000, "Send telemetry", 1, 0, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(ark_tsync_task, "ARK time sync", configMINIMAL_STACK_SIZE + 4000, "ARK time sync", 1, 0, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(imi_msg_rcv_task, "IMI rcv", configMINIMAL_STACK_SIZE + 4000, 0, 1, 0, tskNO_AFFINITY);


    printf("Wow\n");
}


static void task_send_telemetry(void *pvParameters) {
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



