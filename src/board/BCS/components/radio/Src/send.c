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

#define RADIO_SEND_DELAY 50


typedef struct  {
	int id; //ID сообщения
	float period; //Период отправки сообщений (в кол-ве сообщений)
	int last; //Номер сообщения, когда был отправлен
	int is_updated; //Обновлен ли после последнего отправления
	mavlink_message_t last_msg; //Сообщение на отправку
} msg_container;

/*
 * Буфер всех заданных сообщений
 */
#define F(x,a,b) {a, b, -b, 0, {0}},
static msg_container arr_id[] = {
		RADIO_SEND_ID_ARRAY(F)
};
#undef F

//Устанавливаем первое last в ноль, чтобы в начале отпрвлялись
//только нормальные сообщения
msg_container default_msg = {0, RADIO_DEFAULT_PERIOD, 0, 0, {0}};


/*
 * Хэш-функция для получения индекса в массиве arr_id для заданного
 * номера сообщения.
 * Работает через switch/case. В теории C компилятор способен
 * оптимизировать это при больших кол-ах case. Поэтому через
 * define определенно switch/case выражение, которое возвращает
 * индекс в массиве arr_id для msgid данного сообщения.
 */
static int get_hash(int id) {
#define F(x,a,b) case a: return x;
	switch(id) {
	RADIO_SEND_ID_ARRAY(F)
	default:
		return -1;
	}
#undef F
}

#define RADIO_SEND_BUF_SIZE 30
static msg_container arr_buf[RADIO_SEND_BUF_SIZE];
static int arr_buf_size = 0;

/*
 * Поиск контейнера для данного сообщения в буфере arr_buf
 */
static msg_container* find(const mavlink_message_t *msg) {
	for (int i = 0; i < arr_buf_size; i++) {
		if (arr_buf[i].last_msg.msgid == msg->msgid &&
			arr_buf[i].last_msg.compid == msg->compid &&
			arr_buf[i].last_msg.sysid == msg->sysid) {
			return  &arr_buf[i];
		}
	}
	return 0;
}
/*
 * Создание контейнера в буфере arr_buf для данного сообщения
 */
static int add(const mavlink_message_t *msg) {
	if (arr_buf_size >= RADIO_SEND_BUF_SIZE) {
		ESP_LOGE("radio", "No free space for new msg");
		return 1;
	}
	ESP_LOGI("radio", "Add: %d %d:%d", msg->msgid, msg->sysid, msg->compid);
#define F(a) case a: return 2;
	switch (msg->msgid) {
	RADIO_SEND_BAN(F)
	default: break;
	}
#undef F
	int id = get_hash(msg->msgid);
	if (id >= 0) {
		arr_buf_size++;
		arr_buf[arr_buf_size - 1] = arr_id[id];
	} else {
		arr_buf_size++;
		arr_buf[arr_buf_size - 1].id = msg->msgid;
		arr_buf[arr_buf_size - 1].period = RADIO_DEFAULT_PERIOD;
		arr_buf[arr_buf_size - 1].last = -RADIO_DEFAULT_PERIOD;
	}
	return 0;
}

/*
 * Обновление сообщения в соответствующем контейнере из буфера arr_buf.
 * Если контейнера нет - создает новый.
 */
static void update_msg(const mavlink_message_t *msg) {
	msg_container *st = find(msg);
	if (!st) {
		if (add(msg)) {
			return;
		}
		st = &arr_buf[arr_buf_size - 1];
	}
	st->last_msg = *msg;
	st->is_updated = 1;
}
/*
 * Коэффициент важности/срочности/хорошести данного сообщения.
 * Больше - важнее.
 */
static float get_coef(const msg_container *a, int now) {
	//assert(a->period > 0);
	return (now - a->last) / a->period;
}
/*
 * Отбирает самое важное/срочное/лучшее сообщение для отправки.
 * now - количество отпраленных сообщений до этого момента.
 */
static msg_container *get_best(int now) {
	msg_container *st_best = 0;
	float coef_best = 0;
	for (int i = 0; i < arr_buf_size; i++) {
		float coef = get_coef(&arr_buf[i], now);
		if (coef > coef_best && arr_buf[i].is_updated) {
			st_best = &arr_buf[i];
			coef_best = coef;
		}
	}
	if (st_best) {
		ESP_LOGI("radio", "chosen %d with coef %f, period %f, last %d, now %d",
				st_best->id, coef_best, st_best->period, st_best->last, now);
	}
	return st_best;
}

static void task_recv(void *arg) {
	its_rt_task_identifier tid;
	//Регистрируем на сообщения всех типов
	tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN);
	its_rt_register_for_all(tid);


	while (1) {
		mavlink_message_t msg;
		//Ожидаем получения сообщения
		xQueueReceive(tid.queue, &msg, portMAX_DELAY);

		update_msg(&msg);
	}
}
/*
 * Настройки safe_uart_send
 */
typedef struct  {
	uint32_t baud_send; //Баудрейт устр-ва в бит/сек
	uint32_t buffer_size; //Размер буфера внутри устр-ва
	uart_port_t port; //Порт уарта
	uint32_t low_thrld; //Нижная граница буфера
	uint32_t high_thrld;//Верхняя граница буфера
} safe_send_cfg_t;

/*
 * Параметры safe_uart_send
 */
typedef struct  {
	safe_send_cfg_t cfg; //Настройки
	int32_t filled;	//Заполненность буферв
	int64_t last_checked; //Время в мс последнего изменения filled
} safe_send_t;

/*
 * Безопасная отправка данных через уарт. Необходимо инициализировать
 * h->cfg.
 *
 * Безопасность заключается в отпрвке данных с такой частотой, чтобы
 * не превысить отправляющую способность устройства по ту сторону от
 * уарта.
 *
 * Границы буфера позволяют отправлять данные пачками
 */
static void safe_uart_send(safe_send_t *h, uint8_t *buf, uint16_t size) {
	uint32_t Bs = h->cfg.baud_send / 8; //Перевод из бит/сек в Байт/сек

	//Буфер успел освободиться за то время, пока эта ф-ия не вызывалась
	int64_t now = esp_timer_get_time();
	h->filled -= ((now - h->last_checked) * Bs) / 1000000;
	h->filled = h->filled > 0 ? h->filled : 0;


	while (size > 0) {
		if (h->filled >= h->cfg.high_thrld) {
			//Если буфер достаточно заполнен, то можно пока не отправлять.
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
static void task_send(void *arg) {
	safe_send_t sst = {0};
	sst.cfg.low_thrld = 0;
	sst.cfg.high_thrld = 100;
	sst.cfg.baud_send = 720;
	sst.cfg.buffer_size = 1000;
	sst.cfg.port = ITS_UART0_PORT;


	//Количество отправленных сообщений
	int msg_count = 0;
	while (1) {

		msg_container *st = 0;
		while (1) {
			st = get_best(msg_count);

			if (st == 0) {
				//Нет сообщений, которые можно было бы отправить! Подождем...
				vTaskDelay(RADIO_SEND_DELAY / portTICK_PERIOD_MS);
			} else {
				break;
			}
		}
		uint8_t buf[MAVLINK_MAX_PACKET_LEN] = {0};
		int count = mavlink_msg_to_send_buffer(buf, &st->last_msg);
		st->is_updated = 0; //Сообщение внутри контейнера уже не свежее
		st->last = msg_count;//Запоминаем, когда сообщение было отправленно в последний раз
		safe_uart_send(&sst, buf, count);

		msg_count++;
	}
	vTaskDelete(NULL);
}

void radio_send_init(void) {

	xTaskCreatePinnedToCore(task_send, "Radio send", configMINIMAL_STACK_SIZE + 4000, 0, 2, 0, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(task_recv, "Radio buf", configMINIMAL_STACK_SIZE + 4000, 0, 2, 0, tskNO_AFFINITY);
}
