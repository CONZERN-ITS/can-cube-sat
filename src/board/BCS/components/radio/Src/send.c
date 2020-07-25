/*
 * send.c
 *
 *  Created on: Jul 24, 2020
 *      Author: sereshotes
 */


#include "radio.h"
#include "init_helper.h"
#include "router.h"
#include "assert.h"
#include "esp_log.h"


typedef struct  {
	int id; //ID сообщения
	float period; //Период отправки сообщений (в кол-ве сообщений)
	int last; //Номер сообщения, когда был отправлен
	int is_updated; //Обновлен ли после последнего отправления
	mavlink_message_t last_msg;
} send_type;

/*
 * Буфер всех заданных сообщений
 */
#define F(x,a,b) {a, b, -b, 0, {0}},
static send_type arr_id[] = {
		RADIO_SEND_ID_ARRAY(F)
};
#undef F



static int get_hash(int id) {
#define F(x,a,b) case a: return x;
	switch(id) {
	RADIO_SEND_ID_ARRAY(F)
	default:
		return -1;
	}
#undef F
}


//Устанавливаем первое last в ноль, чтобы в начале отпрвлялись
//только нормальные сообщения
send_type default_msg = {0, RADIO_DEFAULT_PERIOD, 0, 0, {0}};

static void task_recv(void *arg) {
	its_rt_task_identifier tid;
	tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN);
	its_rt_register_for_all(tid);


	while (1) {
		mavlink_message_t msg;
		xQueueReceive(tid.queue, &msg, portMAX_DELAY);

		int hash = get_hash(msg.msgid);
		if (hash >= 0) {
			arr_id[hash].last_msg = msg;
			arr_id[hash].is_updated = 1;
		} else {
#define F(a) case a:
			switch (msg.msgid) {
			RADIO_SEND_BAN(F)	break;
			default:
				default_msg.last_msg = msg;
				default_msg.is_updated = 1;
				break;
			}
#undef F
		}
	}
}

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
static void safe_uart_send(safe_send_t *h, uint8_t *buf, uint16_t size) {
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
static float get_coef(const send_type *a, int now) {
	//assert(a->period > 0);
	return (now - a->last) / a->period;
}
static send_type *get_best(int now) {
	send_type *st_best = 0;
	float coef_best = 0;
	if (default_msg.is_updated) {
		st_best = &default_msg;
		coef_best = get_coef(&default_msg, now);
	}

	for (int i = 0; i < sizeof(arr_id) / sizeof(arr_id[0]); i++) {
		float coef = get_coef(&arr_id[i], now);
		if (coef > coef_best && arr_id[i].is_updated) {
			st_best = &arr_id[i];
			coef_best = coef;
		}
	}
	if (st_best) {
		ESP_LOGI("radio", "chosen %d with coef %f, period %f, last %d, now %d",
				st_best->id, coef_best, st_best->period, st_best->last, now);
	}
	return st_best;
}
void task_send(void *arg) {
	safe_send_t sst = {0};
	sst.cfg.low_thrld = 0;
	sst.cfg.high_thrld = 300;
	sst.cfg.baud_send = 720;
	sst.cfg.buffer_size = 1000;
	sst.cfg.port = ITS_UART0_PORT;


	//Количество отправленных сообщений
	int msg_count = 0;
	while (1) {

		send_type *st = 0;
		while (1) {
			st = get_best(msg_count);

			if (st == 0) {
				vTaskDelay(5);
			} else {
				break;
			}
		}
		uint8_t buf[MAVLINK_MAX_PACKET_LEN] = {0};
		int count = mavlink_msg_to_send_buffer(buf, &st->last_msg);
		st->is_updated = 0;
		st->last = msg_count;
		safe_uart_send(&sst, buf, count);
		msg_count++;
	}
	vTaskDelete(NULL);
}

void radio_send_init(void) {

	xTaskCreatePinnedToCore(task_send, "Radio send", configMINIMAL_STACK_SIZE + 4000, 0, 2, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(task_recv, "Radio buf", configMINIMAL_STACK_SIZE + 4000, 0, 2, 0, tskNO_AFFINITY);
}
