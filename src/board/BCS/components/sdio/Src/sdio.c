#include "sdio.h"

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <errno.h>
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

#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

#define SD_RETRY_DELAY 10 //ms
#define SD_WRITE_PERIOD 10000 //ms
static uint64_t last_sync;

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

static void sd_task_telemetry(void *arg);

static void reboot(void);

sdmmc_host_t host = SDMMC_HOST_DEFAULT();
static sdmmc_card_t* card;
const char mount_point[] = MOUNT_POINT;

static void sd_task_test(void *arg);

int sd_init(void) {

	xTaskCreatePinnedToCore(sd_task_test, "Sd task", configMINIMAL_STACK_SIZE + 4000, 0, 4, 0, tskNO_AFFINITY);
	return 0;/*
	esp_err_t ret;
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
	ESP_LOGI(TAG, "Initializing SD card");

	ESP_LOGI(TAG, "Using SDMMC peripheral");

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
	slot_config.width = 4;
	gpio_config_t init_pin_cmd = {
		.mode = GPIO_MODE_OUTPUT_OD,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
		.pin_bit_mask = 1ULL << 15
	};

	gpio_config(&init_pin_cmd);
	gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
	gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
	gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
	gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
	gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

	ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
				"If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
				"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return ret;
	}
	xTaskCreatePinnedToCore(sd_task_telemetry, "Sd task", configMINIMAL_STACK_SIZE + 4000, 0, 4, 0, tskNO_AFFINITY);
	last_sync = esp_timer_get_time();
	return 0;*/
}
static void reboot(void) {
	ESP_LOGD("SD", "rebooting...");
	// All done, unmount partition and disable SDMMC or SPI peripheral
	esp_vfs_fat_sdcard_unmount(mount_point, card);
	ESP_LOGI(TAG, "Card unmounted");
	sd_init();
}
static int id = 0;
typedef enum {
	SD_STATE_UNMOUNTED,
	SD_STATE_MOUNTED_UNOPEN,
	SD_STATE_OPEN_WRITING,
} sd_state_t;
static void sd_task_test(void *arg) {
	sd_state_t state = SD_STATE_UNMOUNTED;

	gpio_config_t init_pin_cmd = {
		.mode = GPIO_MODE_OUTPUT_OD,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
		.pin_bit_mask = 1ULL << 15
	};
	gpio_config(&init_pin_cmd);
	gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
	gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
	gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
	gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
	gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

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

	its_rt_task_identifier tid = {
			.name = "sd_send"
	};
	while (1) {
		//Регистрируем на сообщения всех типов
		if (!(tid.queue = xQueueCreate(10, MAVLINK_MAX_PACKET_LEN)) || its_rt_register_for_all(tid)) {
			ESP_LOGE("SD", "Not enough memory");
			vTaskDelay(SD_RETRY_DELAY / portTICK_PERIOD_MS);
		} else {
			break;
		}
	}


	int fout = 0;
	int id = 0;
	while (1) {
		switch (state) {
		case SD_STATE_UNMOUNTED: {
			esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
			if (!ret) {
				state = SD_STATE_MOUNTED_UNOPEN;
			} else {
				vTaskDelay(SD_RETRY_DELAY / portTICK_PERIOD_MS);
			}
		} break;
		case SD_STATE_MOUNTED_UNOPEN: {

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
				ESP_LOGD("SD", "File created");
				state = SD_STATE_OPEN_WRITING;
			}
		} break;
		case SD_STATE_OPEN_WRITING: {

			mavlink_message_t msg = {0};
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
				uint8_t buf[MAVLINK_MAX_PACKET_LEN] = {0};
				int size = mavlink_msg_to_send_buffer(buf, &msg);
				uint8_t *p = buf;
				while (size > 0) {
					int count = write(fout, p, size);
					if (count < 0) {
						close(fout);
						fout = -1;
						state = SD_STATE_MOUNTED_UNOPEN;
						ESP_LOGE("SD", "write error: %d", errno);
						vTaskDelay(SD_RETRY_DELAY / portTICK_PERIOD_MS);
						break;
					}
					p += count;
					size -= count;
				}
			}
			if (fout >= 0 && esp_timer_get_time() - last_sync > SD_WRITE_PERIOD * 1000ull) {
				last_sync = esp_timer_get_time();
				ESP_LOGD("SD", "Saving in sd");
				int er = fsync(fout);
				ESP_LOGD("SD", "Saved in sd");
				if (er) {
					ESP_LOGE("SD", "Can't sync: %d", errno);
				}
			}
		} break;
		}
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
