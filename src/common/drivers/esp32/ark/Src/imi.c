#include "imi.h"
#include "pinout_cfg.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


////////////////////////////////////////////////////////////////
//Basic driver
////////////////////////////////////////////////////////////////

int imi_send_cmd(imi_t *himi, imi_cmd_t cmd) {
	return my_i2c_send(himi->i2c_port, himi->address, (uint8_t*) &cmd, 1, himi->timeout);
}

int imi_msg_send(imi_t *himi, uint8_t *data, uint16_t size) {
	esp_err_t err = imi_send_cmd(himi, IMI_CMD_SET_PACKET);
	if (err) {
		printf("Error: send cmd1 = %d\n", (int)err);
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
		printf("Error: send cmd2 = %d\n", (int)err);
		return err;
	}

	return 0;
}



int imi_get_packet_size(imi_t *himi, uint16_t *size) {
	int err = imi_send_cmd(himi, IMI_CMD_GET_SIZE);
	if (err) {
		printf("Error: send cmd1 = %d\n", (int)err);
		return err;
	}


	uint16_t rsize = 0;
	err = my_i2c_recieve(himi->i2c_port, himi->address, (uint8_t*) &rsize, sizeof(rsize), himi->timeout);
	if (err) {
		printf("Error: send cmd2 = %d\n", (int)err);
		return err;
	}
	*size = rsize;
	return 0;
}
int imi_get_packet(imi_t *himi, uint8_t *data, uint16_t size) {
	int err = imi_send_cmd(himi, IMI_CMD_GET_PACKET);
	if (err) {
		printf("Error: send cmd3 = %d\n", (int)err);
		return err;
	}

	err = my_i2c_recieve(himi->i2c_port, himi->address, data, size, himi->timeout);
	if (err) {
		printf("Error: send cmd4 = %d\n", (int)err);
		return err;
	}
	return 0;
}
int imi_msg_recieve(imi_t *himi, uint8_t *data, uint16_t *size) {
	uint16_t rsize = 0;
	int rc = 0;
	rc = imi_get_packet_size(himi, &rsize);
	if (rc) {
		return rc;
	}
	if (rsize > *size || rsize == 0) {
		printf("Bad size: %d\n", rsize);
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
imi_t imi_handler[ITS_IMI_DEVICE_COUNT];


typedef struct imi_desc {
	uint8_t* (*salloc)(uint16_t size);				//Static allocation function
	void (*save)(uint8_t *data, uint16_t size);		//Send packet to other systems
	xSemaphoreHandle is_any_read;						//Handles task sleeping
} imi_desc;

static uint8_t *_imi_alloc(uint16_t size) {
	return 0;
}
static void _imi_save(uint8_t *data, uint16_t size) {
}

//Main task task descriptor
static imi_desc _imi_desc = {
		.salloc = _imi_alloc,
		.save = _imi_save
};

void imi_init(void) {
	_imi_desc.is_any_read = xSemaphoreCreateBinary();
}

//If line is pulled, awoke our task immediately
void IRAM_ATTR imi_i2c_int_isr_handler(void* arg) {
	BaseType_t higherWoken = 0;
	xSemaphoreGiveFromISR(_imi_desc.is_any_read, &higherWoken);
	if (higherWoken) {
		portYIELD_FROM_ISR();
	}
}

//Setting external functions
void imi_set_alloc_callback(uint8_t* (*alloc)(uint16_t size)) {
	_imi_desc.salloc = alloc;
}
void imi_set_save_callback(void (*save)(uint8_t *data, uint16_t size)) {
	_imi_desc.save = save;
}

//The main task
static const char *TAG = "imi";
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
}

