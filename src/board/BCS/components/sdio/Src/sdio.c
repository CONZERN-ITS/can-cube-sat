#include "sdio.h"

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#include "soc/sdmmc_periph.h"
#include "soc/sdio_slave_periph.h"
#include "esp_attr.h"
#include "esp_serial_slave_link/essl_sdio.h"
#include "driver/sdmmc_host.h"

#include "router.h"
#include "mavlink_help2.h"
#include "log_collector.h"

#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"

#define MOUNT_POINT "/sdcard"
#define NAME_BASE "T"

#define SD_GLOBAL_MAX_RETRY_COUNT 4
#define SD_GLOBAL_RETRY_DELAY 50
#define SD_MOUNT_MAX_RETRY_COUNT 150
#define SD_MOUNT_DELAY 2000

#define SD_SAVE_RETRY_DELAY 1000 //ms
#define SD_SAVE_PERIOD 10000 //ms
static uint64_t last_sync;

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.





static sdmmc_card_t* card;
static const char mount_point[] = MOUNT_POINT;


static void sd_log_task(void *arg);

static TaskHandle_t sd_t;
static TaskHandle_t sd_l;

static void sd_task(void *arg);
sd_error_t sd_init(void) {
	BaseType_t ret;
	ret = xTaskCreatePinnedToCore(sd_task, "Sd task", configMINIMAL_STACK_SIZE + 3000, 0, 4, &sd_t, tskNO_AFFINITY);
	if (ret != pdTRUE) {
		ESP_LOGE("SD", "can't create task sd_task");
		return SD_ERR0R_LOW_MEMORY;
	}

	ret = xTaskCreatePinnedToCore(sd_log_task, "Sd log task", configMINIMAL_STACK_SIZE + 2000, 0, 1, &sd_l, tskNO_AFFINITY);
	if (ret != pdTRUE) {
		ESP_LOGE("SD", "can't create task sd_log_task");
		return SD_ERR0R_LOW_MEMORY;
	}
	return 0;
}

static sd_error_t sd_error;
static sd_state_t sd_state;
static int sd_error_count;
static int64_t last_time;


static sd_state_t sd_before_sleep_state;
void sd_suspend(void) {
	vTaskSuspend(sd_t);
	sd_before_sleep_state = sd_state;
}

void sd_resume(void) {
	sd_state = sd_before_sleep_state;
	vTaskResume(sd_t);
}

static int _sd_write(int fd, const mavlink_message_t *msg) {
	int ret = 0;
	int test = 0;
#define F(a) case a: break;
	switch (msg->msgid) {
	SD_BAN(F)
	default: test = 1;
	}
#undef F
	if (test) {
		uint8_t buf[MAVLINK_MAX_PACKET_LEN] = {0};
		int size = mavlink_msg_to_send_buffer(buf, msg);
		uint8_t *p = buf;
		while (size > 0) {
			int count = write(fd, p, size);
			if (count < 0) {
				sd_error = SD_ERROR_WRITE;
				ret = -1;
				ESP_LOGE("SD", "write error: %d", errno);
				break;
			}
			p += count;
			size -= count;
		}
	}
	return ret;
}

static int _sd_try_to_save(int fd) {
	int ret = 0;
	if (fd >= 0 && esp_timer_get_time() - last_sync > SD_SAVE_PERIOD * 1000ull) {
		int er = fsync(fd);
		if (!er) {
			last_sync = esp_timer_get_time();
			ESP_LOGD("SD", "Saved in sd");
		} else {
			sd_error = SD_ERROR_SAVE;
			ret = -1;
			ESP_LOGE("SD", "Can't save: %d", errno);
		}
	}
	return ret;
}

static int _sd_find_last(char *dir_name, int *number) {
	int ret = 0;
	struct dirent *pDirent = 0;
	DIR *pDir = 0;
	if ((pDir = opendir(dir_name)) == NULL) {
		sd_error = SD_ERROR_DIR;
		ESP_LOGE("SD", "opendir error: %d", errno);
		return -1;
	}
	int max = 0;
	errno = 0;
	while ((pDirent = readdir(pDir)) != NULL) {
		int t = 0;
		if (sscanf(pDirent->d_name, NAME_BASE"%d", &t) == 1) {
			max = t > max ? t : max;
		}
	}
	if (errno) {
		sd_error = SD_ERROR_DIR;
		ESP_LOGE("SD", "readdir error: %d", errno);
		return -1;
	}
	*number = max + 1;
	ESP_LOGD("SD", "Found next number: %d", *number);
	return 0;

}
static int _sd_open(char *name) {
	int fd = -1;
	fd = open(name, O_WRONLY | O_EXCL | O_CREAT, 0660);
	ESP_LOGD("SD", "Open: %d", errno);
	if (fd < 0) {
		sd_error = SD_ERROR_OPEN;
		ESP_LOGE("SD", "open error: %d", errno);
	} else {
		ESP_LOGD("SD", "File created %s", name);
	}
	return fd;
}
static int _sd_mount_connect() {

	gpio_config_t init_pin_cmd = {
		.mode = GPIO_MODE_OUTPUT_OD,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
		.pin_bit_mask = 1ULL << 15
	};
	gpio_config(&init_pin_cmd);
	//gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
	//gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
	//gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
	//gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
	//gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	host.max_freq_khz = SDMMC_FREQ_DEFAULT;
	host.flags = SDMMC_HOST_FLAG_1BIT;

	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
	slot_config.width = 1;

	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
		.format_if_mount_failed = true,
#else
		.format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
		.max_files = 10,
		.allocation_unit_size = 16 * 1024
	};

	ESP_LOGD("SD","WOW");
	card = 0;
	esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
	ESP_LOGD("SD","WOW1");
	if (ret == ESP_OK) {
		ESP_LOGD("SD", "FS mounted");
		return 0;
	} else {
		sd_error = SD_ERROR_MOUNT;
		ESP_LOGE("SD", "mount error: %d", ret);
		return -1;
	}
}
static void sd_task(void *arg) {
	int sd_retry_count = 0;
	int name_number = -1;
	sd_state = SD_STATE_UNMOUNTED;

	mavlink_message_t msg = {0};
	int is_msg_valid = 0;

	its_rt_task_identifier tid = {
			.name = "sd_send"
	};
	while (1) {
		//Регистрируем на сообщения всех типов
		if (!(tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN)) || its_rt_register_for_all(tid)) {
			ESP_LOGE("SD", "Not enough memory");
			sd_error = SD_ERR0R_LOW_MEMORY;
			vTaskDelay(SD_MOUNT_DELAY / portTICK_PERIOD_MS);
		} else {
			break;
		}
	}

	last_time = esp_timer_get_time();
	int fout = 0;
	while (1) {
cycle:
		switch (sd_state) {
		case SD_STATE_UNMOUNTED: {
			if (sd_retry_count > SD_MOUNT_MAX_RETRY_COUNT) {
				ESP_LOGE("SD", "Have to restart esp32 %d", sd_retry_count);
				ESP_LOGE("SD", "Restarting...");
				sd_retry_count = 0;
				fflush(stdout);
				esp_restart();
			}
			if (_sd_mount_connect()) {
				sd_retry_count++;
				sd_error_count++;
				vTaskDelay(SD_MOUNT_DELAY / portTICK_PERIOD_MS);
				goto cycle;
			} else {
				sd_retry_count = 0;
				sd_state = SD_STATE_MOUNTED_UNOPEN;
			}
		} break;
		case SD_STATE_MOUNTED_UNOPEN: {
			if (sd_retry_count > SD_GLOBAL_MAX_RETRY_COUNT) {
				ESP_LOGE("SD", "Can't open file. Have to reboot SD %d", sd_retry_count);
				sd_retry_count = 0;
				esp_vfs_fat_sdcard_unmount(mount_point, card);
				sd_state = SD_STATE_UNMOUNTED;
				goto cycle;
			}
			if (name_number < 0) {
				if (_sd_find_last(MOUNT_POINT, &name_number)) {
					sd_retry_count++;
					sd_error_count++;
					vTaskDelay(SD_GLOBAL_RETRY_DELAY / portTICK_PERIOD_MS);
					goto cycle;
				} else {
					sd_retry_count = 0;
				}
			}
			char str[200] = {0};
			snprintf(str, sizeof(str), MOUNT_POINT"/"NAME_BASE"%d", name_number);
			if ((fout = _sd_open(str)) < 0) {
				sd_retry_count++;
				sd_error_count++;
				vTaskDelay(SD_GLOBAL_RETRY_DELAY / portTICK_PERIOD_MS);
				goto cycle;
			} else {
				sd_state = SD_STATE_OPEN_WRITING;
				sd_retry_count = 0;
			}
		} break;
		case SD_STATE_OPEN_WRITING: {
			static int retry_write = 0;
			static int retry_save = 0;

			if (retry_write > SD_GLOBAL_MAX_RETRY_COUNT || retry_save > SD_GLOBAL_MAX_RETRY_COUNT) {
				ESP_LOGE("SD", "Can't write anymore. Have to create new file %d %d", retry_write, retry_save);
				retry_write = 0;
				retry_save = 0;
				_sd_try_to_save(fout);
				close(fout);
				name_number++;
				sd_state = SD_STATE_MOUNTED_UNOPEN;
				goto cycle;
			}
			if (!is_msg_valid) {
				//Ожидаем получения сообщения
				xQueueReceive(tid.queue, &msg, portMAX_DELAY);
				is_msg_valid = 1;
			}
			last_time = esp_timer_get_time();

			if (_sd_write(fout, &msg)) {
				sd_error_count++;
				retry_write++;
				vTaskDelay(SD_GLOBAL_RETRY_DELAY / portTICK_PERIOD_MS);
				goto cycle;
			} else {
				retry_write = 0;
				is_msg_valid = 0;
			}
			if (_sd_try_to_save(fout)) {
				sd_error_count++;
				retry_save++;
				vTaskDelay(SD_GLOBAL_RETRY_DELAY / portTICK_PERIOD_MS);
				goto cycle;
			} else {
				retry_save = 0;
			}
		} break;
		default: {
			assert(0); // Мы здесь быть не должны
		}
		}
	}
}

static void sd_log_task(void *arg) {
	while (1) {
		log_data_t ld = {
				.last_error = sd_error,
				.error_count = sd_error_count,
				.last_state = sd_state,
				.ellapsed_time = (esp_timer_get_time() - last_time) / 1000
		};
		log_collector_add(LOG_COMP_ID_SD, &ld);
		vTaskDelay(LOG_COLLECTOR_ADD_PERIOD_COMMON / portTICK_PERIOD_MS);
	}
}

/*
static void sd_task_telemetry(void *arg)
{

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	int fout = -1;
	while (1) {
		char str[200] = {0};
		snprintf(str, sizeof(str), MOUNT_POINT"/T%d", id);
		fout = open(str, O_WRONLY | O_EXCL | O_CREAT, 0660);
		ESP_LOGD("SD", "Open: %d", errno);
		if (fout < 0) {
			if (0) {
				//errno равен 2 всегда, если что-то идет не так.
			} else {
				id++;
				continue;
			}
		} else {
			break;
		}
	}

	its_rt_task_identifier tid = {
			.name = "sd_send"
	};
	//Регистрируем на сообщения всех типов
	tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN);
	its_rt_register_for_all(tid);

	ESP_LOGD("SD", "File created");

	while (1) {
		mavlink_message_t msg;
		//Ожидаем получения сообщения
		xQueueReceive(tid.queue, &msg, portMAX_DELAY);
		int test = 0;
#define F(a) case a: break;
		switch (msg.msgid) {
		SD_BAN(F)
		default: test = 1;
		}
#undef F
		if (test) {
			int size = sizeof(msg);
			uint8_t *buf = (uint8_t *)&msg;
			while (size > 0) {
				int count = write(fout, buf, size);
				if (count < 0) {
					close(fout);
					ESP_LOGE("SD", "write error: %d", errno);
					vTaskDelay(SD_RETRY_DELAY / portTICK_PERIOD_MS);
					its_rt_unregister_for_all(tid);
					vQueueDelete(tid.queue);
					reboot();
					vTaskDelete(0);
					//continue;
				}
				buf += count;
				size -= count;
			}
		}
		if (esp_timer_get_time() - last_sync > SD_WRITE_PERIOD * 1000ull) {
			last_sync = esp_timer_get_time();
			ESP_LOGD("SD", "Saving in sd");
			int er = fsync(fout);
			ESP_LOGD("SD", "Saved in sd");
			if (er) {
				ESP_LOGE("SD", "Can't sync: %d", errno);
			}
		}
	}



	// All done, unmount partition and disable SDMMC or SPI peripheral
	esp_vfs_fat_sdcard_unmount(mount_point, card);
	ESP_LOGI(TAG, "Card unmounted");
	vTaskDelete(0);
}*/
