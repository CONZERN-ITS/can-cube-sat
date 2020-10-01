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
#define F(__msg_hash, __msg_id, __period) {__msg_id, __period, -__period, 0, {0}},
static const msg_container arr_id[] = {
		RADIO_SEND_ID_ARRAY(F)
};
#undef F

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

//! Проверка на то, что сообщение забанено на отправку по радио
/*! Вернет 0, если не забанено и не 0, если забанено */
static int is_msg_banned(int msg_id)
{
#define F(__msg_id) case __msg_id: return 1;
	switch (msg_id) {
	RADIO_SEND_BAN(F)
	default: break;
	}
#undef F

	return 0;
}

#define RADIO_SEND_BUF_SIZE 30
static msg_container arr_buf[RADIO_SEND_BUF_SIZE];
static int arr_buf_size = 0;
static SemaphoreHandle_t buf_mutex;

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

	if (is_msg_banned(msg->msgid)) {
		return 2;
	}

	if (arr_buf_size >= RADIO_SEND_BUF_SIZE) {
		ESP_LOGE("radio", "No free space for new msg");
		return 1;
	}

	ESP_LOGI("radio", "Add: %d %d:%d", msg->msgid, msg->sysid, msg->compid);
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


/*
 * Настройки safe_uart_send
 */
typedef struct  {
	uint32_t baud_send; //Баудрейт устр-ва в бит/сек
	uint32_t buffer_size; //Размер буфера внутри устр-ва
	uart_port_t port; //Порт уарта
	uint32_t low_thrld; //Нижная граница буфера
	uint32_t high_thrld;//Верхняя граница буфера
	float baud_koef;
} safe_send_cfg_t;

/*
 * Параметры safe_uart_send
 */
typedef struct  {
	safe_send_cfg_t cfg; //Настройки
	int filled;	//Заполненность буферв
	int64_t last_checked; //Время в мкс последнего изменения filled
} safe_send_t;

static 	safe_send_t sst = {
	.cfg = {
			.low_thrld = 50,
			.high_thrld = 100,
			.baud_send = 2400 / 2,
			.buffer_size = 1000,
			.port = ITS_UARTR_PORT,
			.baud_koef = 1,
	}
};


static void task_send(void *arg) {
	// Конфигурация отправки пакетов с контроллируемой скоростью

	// Регистрация таска в маршрутизаторе
	its_rt_task_identifier tid = {
			.name = "radio_send"
	};
	//Регистрируем на сообщения всех типов
	tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN);
	its_rt_register_for_all(tid);


	//Количество отправленных сообщений
	int msg_count = 0;
	// Буфер для отправляемого сообщения
	uint8_t out_buf[MAVLINK_MAX_PACKET_LEN] = {0};

	// Текущая отправляемая порция
	uint8_t * portion = out_buf;
	// размер порции
	int portion_size = 0;


	while (1) {
		// Пробуем получить новое сообщение из очереди
		// Блокируемся ненадолго, чтобы равномерно отправлять сообщения из очереди
		mavlink_message_t incoming_msg = {0};
		BaseType_t received = xQueueReceive(tid.queue, &incoming_msg, 200 / portTICK_PERIOD_MS);
		if (pdTRUE == received)
		{
			// Если мы получили сообщение - складываем в его хранилище
			update_msg(&incoming_msg);
		}

		// Нам есть чего отправлять?
		if (0 == portion_size)
		{
			// Нет, нету. Готовим.
			msg_container *st = 0;
			st = get_best(msg_count);
			if (0 == st)
			{
				// Нет сообщений, Пойдем получать следующее
				ESP_LOGI("radio", "message buffer is empty");
				continue;
			}

			portion_size = mavlink_msg_to_send_buffer(out_buf, &st->last_msg);
			portion = out_buf;

			st->is_updated = 0; // Сообщение внутри контейнера уже не свежее
			st->last = msg_count;// Запоминаем, когда сообщение было отправленно в последний раз
			msg_count++;
		}

		// Отлично, мы готовы отправлять. Сколько там радио может принять?
		const int Bs = sst.cfg.baud_send * sst.cfg.baud_koef / 8; //Перевод из бит/сек в Байт/сек
		//Буфер успел освободиться за то время, пока эта ф-ия не вызывалась
		const int64_t now = esp_timer_get_time();
		// Сколько байт успело уйти из буфера радио
		int64_t drained_bytes = ((now - sst.last_checked) * Bs) / (1000*1000);
		if (drained_bytes >= sst.filled)
			sst.filled = 0;
		else
			sst.filled -= drained_bytes;
		sst.last_checked = now;

		int64_t radio_free_space = sst.cfg.buffer_size - sst.filled;
		if (radio_free_space < 0)
			radio_free_space = 0;

		if (radio_free_space < portion_size)
		{
			// Слишком мало. Подождем. Нужно отправлять целый пакет
			continue;
		}

		// Теперь понятно сколько мы можем и сколько хотим отправить
		int to_write = portion_size;
		if (to_write > radio_free_space)
			to_write = (int)radio_free_space;

		if (to_write > 0)
		{
			int rc = uart_write_bytes(sst.cfg.port, (char*)portion, to_write);
			if (rc < 0)
			{
				// Что-то не то с записью
				ESP_LOGE("radio", "error on uart write: %d", rc);
				// Сбросим этот пакет, так как он всеравно устареет
				portion_size = 0;
				// Пойдем дальше, что делать
				continue;
			}
			else if (rc == 0)
			{
				// Кажется, мы пишем слишком быстро, что фифо за нами не успевает?
				ESP_LOGW("radio", "tx fifo overflow");
				continue;
			}

			// Сдвигаем указатели на количество записанного
			portion += rc;
			portion_size -= rc;

			// эти байты как заполненные в буфере радио
			sst.filled += rc;
		}
	} // while(1)
	vTaskDelete(NULL);
}


static TaskHandle_t task_s = NULL;


void radio_send_suspend(void) {
	if (task_s != NULL)
		vTaskSuspend(task_s);
}


void radio_send_resume(void) {
	if (task_s != NULL)
		vTaskResume(task_s);
}

void radio_send_set_baud_koef(float koef) {
	sst.cfg.baud_koef = koef;
}

void radio_send_init(void) {
	xTaskCreatePinnedToCore(task_send, "Radio send", configMINIMAL_STACK_SIZE + 4000, 0, 4, &task_s, tskNO_AFFINITY);
}
