/*
 * MIT License
 *
 * Copyright (c) 2017 David Antliff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sensors.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "pinout_cfg.h"
#include "router.h"
#include "mavlink_help2.h"

#include "ds18b20.h"

static void sensors_task(void *arg);
#define SAMPLE_PERIOD 1000
#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_12_BIT)
static char *TAG = "SENSORS";

esp_err_t sensors_init(void) {
	xTaskCreatePinnedToCore(sensors_task, "Sensors task", configMINIMAL_STACK_SIZE + 3000, 0, 1, 0, tskNO_AFFINITY);
	return 0;
}
//#define USE_GPIO
static void sensors_task(void *arg) {
	OneWireBus * owb = 0;
#ifdef USE_GPIO
	owb_gpio_driver_info gpio_driver_info = {0};
	vTaskDelay(2000/portTICK_PERIOD_MS);
	owb = owb_gpio_initialize(&gpio_driver_info, ITS_PIN_OWB);
	vTaskDelay(2000/portTICK_PERIOD_MS);
#else
	owb_rmt_driver_info rmt_driver_info = {0};
#ifndef ITS_ESP_DEBUG
	owb = owb_rmt_initialize(&rmt_driver_info, ITS_PIN_OWB, RMT_CHANNEL_1, RMT_CHANNEL_0);
#endif
#endif
	owb_use_crc(owb, true);  // enable CRC check for ROM code

	// Find all connected devices
	ESP_LOGD(TAG, "Find devices:");
	OneWireBus_ROMCode device_rom_codes[ITS_OWB_MAX_DEVICES] = {0};
	int num_devices = 0;
	OneWireBus_SearchState search_state = {0};
	bool found = false;
	owb_search_first(owb, &search_state, &found);
	while (found)
	{
		char rom_code_s[17];
		owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
		ESP_LOGD(TAG, "  %d : %s", num_devices, rom_code_s);
		device_rom_codes[num_devices] = search_state.rom_code;
		++num_devices;
		owb_search_next(owb, &search_state, &found);
	}
	ESP_LOGD(TAG, "Found %d device%s", num_devices, num_devices == 1 ? "" : "s");
	for (int i = 0; i < num_devices; i++)
	{
		ESP_LOGI(
			TAG,
			"ds18b20_addr: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
			(int)device_rom_codes[i].bytes[0],
			(int)device_rom_codes[i].bytes[1],
			(int)device_rom_codes[i].bytes[2],
			(int)device_rom_codes[i].bytes[3],
			(int)device_rom_codes[i].bytes[4],
			(int)device_rom_codes[i].bytes[5],
			(int)device_rom_codes[i].bytes[6],
			(int)device_rom_codes[i].bytes[7]
		);
	}

	// In this example, if a single device is present, then the ROM code is probably
	// not very interesting, so just print it out. If there are multiple devices,
	// then it may be useful to check that a specific device is present.

	if (0)
	{
		// For a single device only:
		OneWireBus_ROMCode rom_code;
		owb_status status = owb_read_rom(owb, &rom_code);
		if (status == OWB_STATUS_OK)
		{
			char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
			owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
			ESP_LOGD(TAG, "Single device %s present", rom_code_s);
		}
		else
		{
			ESP_LOGE(TAG, "An error occurred reading ROM code: %d", status);
		}
	}
	else
	{
		// Search for a known ROM code (LSB first):
		// For example: 0x1502162ca5b2ee28
		OneWireBus_ROMCode known_device = {
			.fields.family = { 0x28 },
			.fields.serial_number = { 0xee, 0xb2, 0xa5, 0x2c, 0x16, 0x02 },
			.fields.crc = { 0x15 },
		};
		char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
		owb_string_from_rom_code(known_device, rom_code_s, sizeof(rom_code_s));
		bool is_present = false;

		owb_status search_status = owb_verify_rom(owb, known_device, &is_present);
		if (search_status == OWB_STATUS_OK)
		{
			ESP_LOGD(TAG, "Device %s is %s", rom_code_s, is_present ? "present" : "not present");
		}
		else
		{
			ESP_LOGE(TAG, "An error occurred searching for known device: %d", search_status);
		}
	}

	// Create DS18B20 devices on the 1-Wire bus
	DS18B20_Info * devices[ITS_OWB_MAX_DEVICES] = {0};
	for (int i = 0; i < num_devices; ++i)
	{
		DS18B20_Info * ds18b20_info = ds18b20_malloc();  // heap allocation
		devices[i] = ds18b20_info;
/*
		if (num_devices == 1)
		{
			ESP_LOGD(TAG, "Single device optimisations enabled");
			ds18b20_init_solo(ds18b20_info, owb);          // only one device on bus
		}
		else
		{*/
			ds18b20_init(ds18b20_info, owb, device_rom_codes[i]); // associate with bus and device
		//}
		ds18b20_use_crc(ds18b20_info, true);           // enable CRC check on all reads
		ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
	}

//    // Read temperatures from all sensors sequentially
//    while (1)
//    {
//        printf("\nTemperature readings (degrees C):\n");
//        for (int i = 0; i < num_devices; ++i)
//        {
//            float temp = ds18b20_get_temp(devices[i]);
//            printf("  %d: %.3f\n", i, temp);
//        }
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
//    }

	// Check for parasitic-powered devices
	bool parasitic_power = false;
	ds18b20_check_for_parasite_power(owb, &parasitic_power);
	if (parasitic_power) {
		ESP_LOGD(TAG, "Parasitic-powered devices detected");
	}

	// In parasitic-power mode, devices cannot indicate when conversions are complete,
	// so waiting for a temperature conversion must be done by waiting a prescribed duration
	owb_use_parasitic_power(owb, 0);

#ifdef CONFIG_ENABLE_STRONG_PULLUP_GPIO
	// An external pull-up circuit is used to supply extra current to OneWireBus devices
	// during temperature conversions.
	owb_use_strong_pullup_gpio(owb, CONFIG_STRONG_PULLUP_GPIO);
#endif

	// Read temperatures more efficiently by starting conversions on all devices at the same time
	int errors_count[ITS_OWB_MAX_DEVICES] = {0};
	int sample_count = 0;
	ESP_LOGD(TAG, "Start cycle");
	if (num_devices > 0)
	{
		TickType_t last_wake_time = xTaskGetTickCount();

		while (1)
		{
			last_wake_time = xTaskGetTickCount();

			ds18b20_convert_all(owb);

			// In this application all devices use the same resolution,
			// so use the first device to determine the delay
			ds18b20_wait_for_conversion(devices[0]);
			struct timeval tv = {0};
			gettimeofday(&tv, 0);

			// Read the results immediately after conversion otherwise it may fail
			// (using printf before reading may take too long)
			float readings[ITS_OWB_MAX_DEVICES] = { 0 };
			DS18B20_ERROR errors[ITS_OWB_MAX_DEVICES] = { 0 };

			for (int i = 0; i < num_devices; ++i)
			{
				errors[i] = ds18b20_read_temp(devices[i], &readings[i]);
			}

			for (int i = 0; i < num_devices; ++i) {
				mavlink_thermal_state_t mts = {0};
				mavlink_message_t msg = {0};
				mts.time_s = tv.tv_sec;
				mts.time_us = tv.tv_usec;
				mts.temperature = errors[i] == DS18B20_OK ? readings[i] : NAN;
				mavlink_msg_thermal_state_encode(mavlink_system, i, &msg, &mts);
				its_rt_sender_ctx_t ctx = {0};

				ctx.from_isr = 0;
				its_rt_route(&ctx, &msg, 0);
			}

			vTaskDelayUntil(&last_wake_time, SAMPLE_PERIOD / portTICK_PERIOD_MS);
		}
	}
	else
	{
		ESP_LOGD(TAG, "No DS18B20 devices detected!");
	}

	// clean up dynamically allocated data
	for (int i = 0; i < num_devices; ++i)
	{
		ds18b20_free(&devices[i]);
	}
	owb_uninitialize(owb);

	vTaskDelete(0);
}
