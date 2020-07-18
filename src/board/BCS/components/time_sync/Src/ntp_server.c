/*
 * ntp_server.c
 *
 *  Created on: Jul 18, 2020
 *      Author: sereshotes
 */


#include "ntp_server.h"


#include "esp_netif.h"
#include "esp_bit_defs.h"
#include "esp_log.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

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
