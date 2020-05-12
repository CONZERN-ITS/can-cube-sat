#include "imi.h"
#include "pinout_cfg.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


////////////////////////////////////////////////////////////////
//Basic driver
////////////////////////////////////////////////////////////////

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

static int imi_get_packet(imi_t *himi, uint8_t *data, uint16_t size) {
	int err = imi_send_cmd(himi, IMI_CMD_GET_PACKET);
	if (err) {
		//printf("Error: send cmd3 = %d\n", (int)err);
		return err;
	}

	err = my_i2c_recieve(himi->i2c_port, himi->address, data, size, himi->timeout);
	if (err) {
		//printf("Error: send cmd4 = %d\n", (int)err);
		return err;
	}
	return 0;
}

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
	xSemaphoreHandle is_any_read;						//Handles task sleeping
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
static void IRAM_ATTR imi_i2c_int_isr_handler(void* arg) {
	imi_handler_t *h = (imi_handler_t *)arg;
	BaseType_t higherWoken = 0;
	vTaskNotifyGiveFromISR(h->taskRecv, &higherWoken);
	if (higherWoken) {
		portYIELD_FROM_ISR();
	}
}

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
				ESP_LOGE(TAG, "ERROR: IMI static alloc error\n");
				continue;
			}
			rc = imi_get_packet(&himi, pointer, size);
			(*h->cfg.save)(pointer, size);
		}
	}
}

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

void imi_install(imi_config_t *cfg, int port) {
	assert(port < IMI_COUNT);
	imi_handler_t *h = &imi_device[port];
	assert(h->state == IMI_STATE_STOPED);
	h->cfg = *cfg;
	h->adds = malloc(h->cfg.address_count * sizeof(*h->adds));
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

void imi_add_address(int port, uint8_t address) {
	imi_handler_t *h;

	assert(port < IMI_COUNT);
	h = &imi_device[port];
	assert(h->state == IMI_STATE_INSTALLED);
	assert(h->add_count < h->cfg.address_count);

	h->adds[h->add_count++] = address;
}

/*

//The main task
void imi_msg_rcv_task(void *pv) {
	while (1) {
		//If line is pulled, take semaphore to wait for the moment when line is pulled
		//Also we try to test line every IMI_WAIT_DELAY to verify we haven't pass line pulling
		while (gpio_get_level(ITS_PIN_I2C_INT)) {
			xSemaphoreTake(_imi_desc.is_any_read, IMI_WAIT_DELAY / portTICK_PERIOD_MS);
		}

		//Receiving packets if there are any
		for (int i = 0; i < ITS_IMI_DEVICE_COUNT; i++) {
			int isAny = 1;
			//If device has a packet, we will try to get one more. So, we will all packets
			//one device, then from another and so on.
			while (isAny) {
				uint16_t size = 0;

				//Getting size. size == 0 means that the device has no packets
				int rc = imi_get_packet_size(&imi_handler[i], &size);
				if (rc || size == 0) {
					isAny = 0;
					continue;
				}
				//Get space for paket
				uint8_t *pointer = (*_imi_desc.salloc)(size);
				if (!pointer) {
					ESP_LOGE(TAG, "ERROR: IMI static alloc error\n");
					continue;
				}
				imi_get_packet(&imi_handler[i], pointer, size);
				(*_imi_desc.save)(pointer, size);
			}
		}
		vTaskDelay(IMI_CYCLE_DELAY / portTICK_PERIOD_MS);
	}
}*/

