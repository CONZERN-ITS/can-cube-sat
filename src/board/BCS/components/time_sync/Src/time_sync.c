/*
 * time_sync.c
 *
 *  Created on: Jul 14, 2020
 *      Author: sereshotes
 */
/*
 * Необходимо иметь службу времени, которая будет
 * синхронизироваться либо с SINS через uart/mavlink и pps,
 * либо с BCS через wifi/ntp. В этом модуле представлены
 * функции для установки службы для любой из этих задач.
 */
#include "time_sync.h"
#include "ntp_server.h"

#include "router.h"
#include "pinout_cfg.h"
#include "mavlink/its/mavlink.h"

#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"






//static void adjust(struct timeval *t1, struct timeval *t2)

void sntp_notify(struct timeval *tv);

/*
 *	Таск, синхронизирующийся с SINS, если пришло
 *	от него сообщение и пришел pps сигнал
 */
static void time_sync_task(void *arg) {
	ts_sync *ts = (ts_sync *)arg;

	its_rt_task_identifier id = {
			.name = "time_sync"
	};
	id.queue = xQueueCreate(3, MAVLINK_MAX_PACKET_LEN);

	//Регистрируем очередь для приема нужных сообщений
	its_rt_register(MAVLINK_MSG_ID_TIMESTAMP, id);
	while (1) {
		mavlink_message_t msg;
		//Ждем получения сообщений
		if (!xQueueReceive(id.queue, &msg, portMAX_DELAY)) {
			//Никогда не должно происходить
			ESP_LOGE("TIME SYNC", "Recieve error");
			vTaskDelay(5000 / portTICK_RATE_MS);
			continue;
		}

		//Пришло ли это от SINS
		if (msg.sysid != CUBE_1_SINS) {
			ESP_LOGI("TIME SYNC","Who is sending it?");
			continue;
		}
		//Был ли pps сигнал
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
				.tv_usec = 0 - ts->here.tv_usec
		};

		ESP_LOGV("TIME", "from sinc: %d.%06d\n", (int)there.tv_sec, (int)there.tv_usec);
		ESP_LOGV("TIME", "here:      %d.%06d\n", (int)ts->here.tv_sec, (int)ts->here.tv_usec);
		/*
		 * Если разница слишком большая, то ставим время мгновенно.
		 * Если нет, то пытаемся приблизить время без больших
		 * скачков
		 */
		if (abs(delta.tv_sec) > TIME_SYNC_SMOOTH_THREASHOLD) {
			ESP_LOGI("TIME SYNC","Too big diff. Immidiate sync!");
			struct timeval t;
			gettimeofday(&t, 0);
			t.tv_sec += delta.tv_sec;
			t.tv_usec += delta.tv_usec;
			settimeofday(&t, 0);
		} else {
			ESP_LOGI("TIME SYNC","Small diff. smooth sync!");
			adjtime(&delta, 0);
		}
		ts->is_updated = 0;
	}

}

/*
 * Функция, вызваемая при опускании линии pps
 */
static void IRAM_ATTR isr_handler(void *arg) {
	ts_sync *ts = (ts_sync *)arg;
	gettimeofday(&ts->here, 0);
	ts->is_updated = 1;
}

/*
 * Устанавливает службу для синхронизации от SINS
 * через uart/mavlink и pps сигнал. В cfg необходимо
 * инициализировать номер пина.
 */
void time_sync_from_sins_install(ts_sync *cfg) {
	cfg->is_updated = 0;
	xTaskCreatePinnedToCore(ntp_server_task, "SNTP server", configMINIMAL_STACK_SIZE + 4000, 0, 1, 0, tskNO_AFFINITY);
	xTaskCreate(time_sync_task, "timesync", 4096, cfg, 1, NULL);
	gpio_config_t init_pin_int = {
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type = GPIO_INTR_POSEDGE,
		.pin_bit_mask = 1ULL << cfg->pin
	};
	gpio_config(&init_pin_int);
	gpio_isr_handler_add(cfg->pin, isr_handler, cfg);
}

/*
 * Устанавливает службу для синхронизации от BCS,
 * который имеет точно время. Работает через wifi/ip/ntp
 */
void time_sync_from_bcs_install(const ip_addr_t *server_ip) {
	sntp_set_time_sync_notification_cb(sntp_notify);
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setserver(0, server_ip);
	sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
	sntp_set_sync_interval(0);
	sntp_init();
	sntp_restart();
	ESP_LOGI("SNTP", "client started");
}



void sntp_notify(struct timeval *tv) {
	ESP_LOGI("SNTP", "We've just synced");
}


