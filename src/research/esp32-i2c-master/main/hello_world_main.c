/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"


#ifdef CONFIG_IDF_TARGET_ESP32
//#define CHIP_NAME "ESP32"
#endif

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */

static gpio_num_t i2c_gpio_sda = 18;
static gpio_num_t i2c_gpio_scl = 19;
static uint32_t i2c_frequency = 50000;
static i2c_port_t i2c_port = I2C_NUM_0;

#define I2C_LINK_PACKET_SIZE (279)
#define LED_GPIO (2)

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

	gpio_pad_select_gpio(LED_GPIO);
	const uint8_t slave_addr = 0x68;

	uint8_t message[I2C_LINK_PACKET_SIZE];
	uint8_t message_in[I2C_LINK_PACKET_SIZE];

	int led_value = 0;
	for (int i = 1; ; i++)
	{
		esp_err_t err;
		memset(message_in, i-1, I2C_LINK_PACKET_SIZE);
		memset(message, i, I2C_LINK_PACKET_SIZE);

		GPIO_OUTPUT_SET(LED_GPIO, led_value);

		i2c_cmd_handle_t cmd =  i2c_cmd_link_create();

		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, slave_addr << 1 | I2C_MASTER_WRITE, 1);
		i2c_master_write(cmd, message, I2C_LINK_PACKET_SIZE, 1);
		i2c_master_stop(cmd);

		err = i2c_master_cmd_begin(i2c_port, cmd, 500 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		printf("err_write = %d\n", (int)err);

		//vTaskDelay(1000 / portTICK_PERIOD_MS);

		cmd =  i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, slave_addr << 1 | I2C_MASTER_READ, 1);
		i2c_master_read(cmd, message_in, I2C_LINK_PACKET_SIZE, I2C_MASTER_LAST_NACK);
		i2c_master_stop(cmd);

		err = i2c_master_cmd_begin(i2c_port, cmd, 500 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		printf("err_read = %d\n", (int)err);


		if (0 == err)
		{
			int rc = memcmp(message, message_in, I2C_LINK_PACKET_SIZE);
			printf("memcmp res = %d\n", rc);
		}

		led_value = !led_value;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	fflush(stdout);
	esp_restart();
}
