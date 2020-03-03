/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#ifdef CONFIG_IDF_TARGET_ESP32
//#define CHIP_NAME "ESP32"
#endif

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */

static gpio_num_t i2c_gpio_sda = 18;
static gpio_num_t i2c_gpio_scl = 19;
static uint32_t i2c_frequency = 100000;
static i2c_port_t i2c_port = I2C_NUM_0;


void app_main(void)
{
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = i2c_gpio_sda,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = i2c_gpio_scl,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = i2c_frequency
	};
	i2c_param_config(i2c_port, &conf);
	i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

	for (int i = 10; i >= 0; i--) {
		printf("Restarting in %d seconds...\n", i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	printf("Restarting now.\n");
	fflush(stdout);
	esp_restart();
}
