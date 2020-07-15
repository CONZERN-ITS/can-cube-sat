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
#include "esp_bit_defs.h"
#include "esp_sntp.h"
#include "esp_netif.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"




//static void adjust(struct timeval *t1, struct timeval *t2)

void ntp_server_task(void *arg);
void sntp_notify(struct timeval *tv);

/*
 *	Таск, синхронизирующийся с SINS, если пришло
 *	от него сообщение и пришел pps сигнал
 */
static void time_sync_task(void *arg) {
	ts_sync *ts = (ts_sync *)arg;

	its_rt_task_identifier id = {0};
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
		if (msg.compid != CUBE_1_SINS) {
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
				.tv_usec = there.tv_usec - ts->here.tv_usec
		};

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
			continue;
		} else {
			adjtime(&delta, 0);
		}
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


//---------------------------------------------------------------------------------
//------------------------------NTP SERVER-----------------------------------------
//---------------------------------------------------------------------------------
#define NTP_BUF_SIZE 12 * 4
#define NTP_PORT 123
#define NTP_CONN_COUNT 4
#define NTP_LI 0
#define NTP_VN 3
#define NTP_MODE 4
#define NTP_STRATUM 1 //Самое неоднозначное поле
#define NTP_POLL 4
#define NTP_PREC -6
#define NTP_UTC_OFFSET 		2208988800L
#define NTP_DELAY 1 << 16
#define NTP_DISP 1 << 16
#define NTP_REFID *((uint32_t *)("GPS"))

/*
 * Таск обслуживает прием ntp запросов и отвечает на
 * каждый из них. Работает как sntp сервер (для понимания,
 * в чем различие с ntp сервером и есть ли вообще это различие
 * см. спецификацию ntp).
 */
void ntp_server_task(void *arg) {
	ESP_LOGI("SNTP", "ntp_server_task: start");
	int sin = socket(AF_INET, SOCK_DGRAM, 0);
	if (sin < 0) {
		ESP_LOGE("SNTP", "Can't create socket");
		vTaskDelete(NULL);
	}
	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(NTP_PORT);

	if (bind(sin, (struct sockaddr *)&addr, sizeof(addr))) {
		ESP_LOGE("SNTP", "Can't bind socket");
		vTaskDelete(NULL);
	}
	ESP_LOGI("SNTP", "ntp_server_task: start reading");

	while (1) {
		uint8_t buf[NTP_BUF_SIZE + 1];
		socklen_t len = sizeof(addr);
		//Пытаемся прочесть чуть больше, чтобы убедиться, что полученное сообщение
		//ровно той же длины, что и ожидаемый пакет
		int size = recvfrom(sin, &buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		if (size < 0) {
			ESP_LOGE("SNTP", "Receive Error: %d", errno);
			continue;
		}
		if (size != NTP_BUF_SIZE) {
			ESP_LOGI("SNTP", "ntp_server_task: not a ntp msg - dumping it");
			continue;
		}
		/*
		 * Авторы NTP определили формат пакета с порядком байт Интернета
		 * в том смысле, что порядок 32-битных слов определяется
		 * спецификацией, а порядок байт внутри слов - BE, как и общий
		 * порядок байт сетях.
		 */
		uint32_t *data = (uint32_t *)buf;

		data[0] = htonl((NTP_LI << 30) | (NTP_VN << 27) | (NTP_MODE << 24) |
				(NTP_STRATUM << 16) | (NTP_POLL << 8) | (NTP_PREC & 0xff)); //Разные флаги
		data[1] = htonl(NTP_DELAY);	//Задержка между двумя обновлениями времени на сервере
		data[2] = htonl(NTP_DISP);	//Дисперсия (см. спец-ию)
		data[3] = htonl(NTP_REFID);	//Идентификатор использумых часов

		struct timeval tm;
		gettimeofday(&tm, 0);

		data[6] = data[10]; //То, что было временем отправлением клиента,
		data[7] = data[11]; //стало начальным временем

		//Время отправки с сервера. Переводим с UTC в NTP.
		//Также это время приема, так как мы не можем иметь
		//что-то лучше, чем это без влезаний в ядро ESP.
		data[8] = data[10] = htonl(tm.tv_sec + NTP_UTC_OFFSET);
		data[9] = data[11] = htonl(((unsigned long long)tm.tv_usec << 32LL) / 1000000.0);
		ESP_LOGI("SNTP", "SEND TO: %d:%d\n", addr.sin_addr.s_addr, addr.sin_port);
		sendto(sin, data, NTP_BUF_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
	}
}

void sntp_notify(struct timeval *tv) {
	ESP_LOGI("SNTP", "We've just synced");
}
