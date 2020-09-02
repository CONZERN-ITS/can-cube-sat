/*
 * Данный модуль IMI (I2C Messaging Interface) есть расширение
 * I2c с помощью дополнительной линии I2C_INT.
 *
 * Зачем: для того, чтобы мастер не опрашивал всех каждые N мс
 * мы используем дополнительную линию, опускаемую одним из
 * подчиненных, если тому есть что сказать.
 *
 * Реализация:
 * 1) На низком уровне есть система команд, отправляемые мастером
 * подчиненным:
 * IMI_CMD_NONE - Нет команды, поведение не определенно
 * IMI_CMD_GET_SIZE - Получить размер сообщения. Если нет сообщения,
 * котоое подчиненный хочет отправить, то отправляется 0
 * IMI_CMD_GET_PACKET - Полчучиь само сообщение
 * IMI_CMD_SET_PACKET - Отправка сообщения. При этом перед отправкой
 * отправляется размер сообщения.
 * Размер - двухбайтовая величина.
 * 2)На высоком уровне
 * каждый может отправить сообщение длиной не больше
 * максимального. При этом подчиненный опускает линию, а мастер
 * спрашивает у каждого, есть ли у того сообщение. Подчиненный
 * отправляет размер сообщения или 0, если нет сообщений. Таким
 * образом, мастер сможет прочитать ровно столько, сколько
 * подчиненный хочет ему отправить (в I2C именно мастер указывает,
 * сколько он хочет прочитать).
 */


#include "imi.h"
#include "pinout_cfg.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


////////////////////////////////////////////////////////////////
//Basic driver
////////////////////////////////////////////////////////////////
/*
 *
 */
static int imi_send_cmd(imi_t *himi, imi_cmd_t cmd) {
	return my_i2c_send(himi->i2c_port, himi->address, (uint8_t*) &cmd, 1, himi->timeout);
}

static int imi_msg_send(imi_t *himi, uint8_t *data, uint16_t size) {
	esp_err_t err = imi_send_cmd(himi, IMI_CMD_SET_PACKET);
	if (err) {
		//printf("Error: send cmd1 = %d\n", (int)err);
		return err;
	}
	//Надо отправить и размер, и пакет. Для этого пришлость воспользоваться низкоуровневыми
	//методами
	i2c_cmd_handle_t cmd =  i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, himi->address << 1 | I2C_MASTER_WRITE, 1);
	i2c_master_write(cmd, (uint8_t*) &size, sizeof(size), 1);
	i2c_master_write(cmd, data, size, 1);
	i2c_master_stop(cmd);
	err = i2c_master_cmd_begin(himi->i2c_port, cmd, himi->timeout / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if (err) {
		//printf("Error: send cmd2 = %d\n", (int)err);
		return err;
	}

	return 0;
}


/*
 * Запрашивает размер сообщения, которое подчиненный хочет отправить
 */
static int imi_get_packet_size(imi_t *himi, uint16_t *size) {
	int err = imi_send_cmd(himi, IMI_CMD_GET_SIZE);
	if (err) {
		//printf("Error: send cmd1 = %d\n", (int)err);
		return err;
	}


	uint16_t rsize = 0;
	err = my_i2c_recieve(himi->i2c_port, himi->address, (uint8_t*) &rsize, sizeof(rsize), himi->timeout);
	if (err) {
		//printf("Error: send cmd2 = %d\n", (int)err);
		return err;
	}
	*size = rsize;
	return 0;
}

/*
 * Запрашивает у подчиненного само сообщение
 */
static int imi_get_packet(imi_t *himi, uint8_t *data, uint16_t size) {
	int err = imi_send_cmd(himi, IMI_CMD_GET_PACKET);
	if (err) {
		printf("Error: send cmd3 = %d\n", (int)err);
		return err;
	}

	err = my_i2c_recieve(himi->i2c_port, himi->address, data, size, himi->timeout);
	if (err) {
		//printf("Error: send cmd4 = %d\n", (int)err);
		return err;
	}
	return 0;
}
/*
 * Заправшивает у подчиненного размер, и если не 0, то запрашивает само
 * сообщение.
 * retval == 0 - нет сообщений у этого подчиненного
 */
static int imi_msg_recieve(imi_t *himi, uint8_t *data, uint16_t *size) {
	uint16_t rsize = 0;
	int rc = 0;
	rc = imi_get_packet_size(himi, &rsize);
	if (rc) {
		return rc;
	}
	if (rsize > *size || rsize == 0) {
		//printf("Bad size: %d\n", rsize);
		return 0;
	}
	*size = rsize;
	rc = imi_get_packet(himi, data, *size);
	return rc;
}


////////////////////////////////////////////////////////////////
//Task driver
////////////////////////////////////////////////////////////////

//All imi devices
//imi_t imi_handler[ITS_IMI_DEVICE_COUNT];


typedef struct imi_desc {

	uint8_t* (*salloc)(uint16_t size);				//Static allocation function
	void (*save)(uint8_t *data, uint16_t size);		//Send packet to other systems
	xSemaphoreHandle is_any_read;					//Handles task sleeping
} imi_desc;

typedef enum {
	IMI_STATE_STOPED,
	IMI_STATE_INSTALLED,
	IMI_STATE_STARTED,
} imi_state_t;

typedef struct {
	imi_config_t cfg;
	TaskHandle_t taskRecv;
	uint8_t *adds;
	int add_count;
	SemaphoreHandle_t mutex;
	imi_state_t state;

} imi_handler_t;

static const char *TAG = "imi";

imi_handler_t imi_device[IMI_COUNT];

//If line is pulled, awoke our task immediately
static void IRAM_ATTR imi_i2c_int_isr_handler(void *arg) {
	imi_handler_t *h = (imi_handler_t *)arg;
	BaseType_t higherWoken = 0;
	vTaskNotifyGiveFromISR(h->taskRecv, &higherWoken);
	if (higherWoken) {
		portYIELD_FROM_ISR();
	}
}

/*
 * Считывает сообщения у всех на линии. Адреса определены
 * в imi_handler_t *h. Сообщения сохраяняются через h->cfg.save.
 */
static void _imi_recv_all(imi_handler_t *h) {

	for (int i = 0; i < h->add_count; i++) {

		int isAny = 1;
		//If device has a packet, we will try to get one more. So, we will all packets
		//one device, then from another and so on.

		while (isAny) {
			uint16_t size = 0;

			//Getting size. size == 0 means that the device has no packets

			imi_t himi = {
				.i2c_port = h->cfg.i2c_port,
				.address = h->adds[i],
				.timeout = h->cfg.ticksToWaitForOne
			};
			int rc = imi_get_packet_size(&himi, &size);
			if (rc || size == 0) {
				isAny = 0;
				continue;
			}
			//Get space for paket
			uint8_t *pointer = (*h->cfg.alloc)(size);
			if (!pointer) {
				ESP_LOGE(TAG, "ERROR: IMI static alloc error %d\n", size);
				/*
				 * Мы хотим больше не читать от этого глупого устройства,
				 * который послал безумно большой пакет
				 */
				isAny = 0;
				continue;
			}
			rc = imi_get_packet(&himi, pointer, size);
			(*h->cfg.save)(pointer, size);
		}

	}
}

/*
 * Таск постоянно проверяет линию на опускание. Если опущена,
 * то считывает сообщение у всех на линии.
 */
static void _imi_task_recv(void *arg) {
	imi_handler_t *h = (imi_handler_t *)arg;

	while (1) {
		//If line is pulled, take semaphore to wait for the moment when line is pulled
		//Also we try to test line every IMI_WAIT_DELAY to verify we haven't pass line pulling
		while (gpio_get_level(h->cfg.i2c_int)) {
			ulTaskNotifyTake(pdFALSE, IMI_WAIT_DELAY / portTICK_RATE_MS);
		}
		//ESP_LOGI(TAG, "Trying to read");
		xSemaphoreTake(h->mutex, portMAX_DELAY);
		//Receiving packets if there are any
		_imi_recv_all(h);
		xSemaphoreGive(h->mutex);

		vTaskDelay(IMI_CYCLE_DELAY / portTICK_RATE_MS);
	}
}

/*
 * Установка imi линии
 */
void imi_install(imi_config_t *cfg, imi_port_t port) {
	assert(port < IMI_COUNT);
	imi_handler_t *h = &imi_device[port];
	assert(h->state == IMI_STATE_STOPED);
	h->cfg = *cfg;
	h->adds = malloc(h->cfg.address_count * sizeof(*h->adds));
	if (!h->adds) {
		ESP_LOGE(TAG, "can't allocate memory");
	}
	h->mutex = xSemaphoreCreateMutex();

	h->state = IMI_STATE_INSTALLED;
}
void imi_install_static(imi_config_t *cfg, imi_port_t port, uint8_t *addrs) {
	assert(port < IMI_COUNT);
	imi_handler_t *h = &imi_device[port];
	assert(h->state == IMI_STATE_STOPED);
	h->cfg = *cfg;
	h->adds = addrs;
	h->add_count = cfg->address_count;
	h->mutex = xSemaphoreCreateMutex();

	h->state = IMI_STATE_INSTALLED;
}
/*
 * Запуск imi линии.
 */
void imi_start(imi_port_t port) {
	assert(port < IMI_COUNT);
	imi_handler_t *h = &imi_device[port];
	assert(h->state == IMI_STATE_INSTALLED);
	char str[100];
	snprintf(str, sizeof(str), "IMI %d recv task", port);
	xTaskCreate(_imi_task_recv, str, 4048, h, 1, &h->taskRecv);

	gpio_config_t init_pin_i2c_int = {
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_NEGEDGE,
		.pin_bit_mask = 1ULL << h->cfg.i2c_int
	};

	gpio_config(&init_pin_i2c_int);
	gpio_isr_handler_add(h->cfg.i2c_int, imi_i2c_int_isr_handler, h);
	h->state = IMI_STATE_STARTED;
}

/*
 * Отправляет сообщение по указанному адресу.
 */
int imi_send(imi_port_t port, uint8_t address, uint8_t *data, uint16_t size, TickType_t ticksToWait) {
	int rc = 0;
	imi_handler_t *h;

	assert(port < IMI_COUNT);
	h = &imi_device[port];
	assert(h->state == IMI_STATE_STARTED);

	imi_t himi = {
		.i2c_port = h->cfg.i2c_port,
		.address = address,
		.timeout = ticksToWait
	};

	if (xSemaphoreTake(h->mutex, ticksToWait) == pdFALSE) {
		return 1;
	}
	rc = imi_msg_send(&himi, data, size);
	xSemaphoreGive(h->mutex);
	return rc;
}
/*
 * Отправляет сообщение по всем адресам.
 */
int imi_send_all(imi_port_t port, uint8_t *data, uint16_t size, TickType_t ticksToWaitForOne) {
	int rc = 0;
	imi_handler_t *h;

	assert(port < IMI_COUNT);
	h = &imi_device[port];
	assert(h->state == IMI_STATE_STARTED);

	if (xSemaphoreTake(h->mutex, ticksToWaitForOne) == pdFALSE) {
		return 1;
	}
	for (int i = 0; i < h->add_count; i++) {

		imi_t himi = {
			.i2c_port = h->cfg.i2c_port,
			.address = h->adds[i],
			.timeout = ticksToWaitForOne
		};
		rc = imi_msg_send(&himi, data, size);
	}

	xSemaphoreGive(h->mutex);
	return rc;
}
/*
 * Добавляет адрес в список адресов. Список заранее создан в виде
 * массива.
 */
void imi_add_address(int port, uint8_t address) {
	imi_handler_t *h;

	assert(port < IMI_COUNT);
	h = &imi_device[port];
	assert(h->state == IMI_STATE_INSTALLED);
	assert(h->add_count < h->cfg.address_count);

	h->adds[h->add_count++] = address;
}

