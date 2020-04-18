/*
 * pin_init.c
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#include "init_helper.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "internet.h"
#include "pinout_cfg.h"
#include "imi.h"
#include "esp_intr_alloc.h"
#include "router.h"
#include "mavlink/its/mavlink.h"

static i2c_config_t init_pin_i2c_tm  = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = ITS_PIN_I2CTM_SDA,
	.sda_pullup_en = GPIO_PULLUP_ENABLE,
	.scl_io_num = ITS_PIN_I2CTM_SCL,
	.scl_pullup_en = GPIO_PULLUP_ENABLE,
	.master.clk_speed = ITS_I2CTM_FREQ
};

static gpio_config_t init_pin_i2c_int = {
	.mode = GPIO_MODE_INPUT,
	.pull_up_en = GPIO_PULLUP_DISABLE,
	.pull_down_en = GPIO_PULLDOWN_DISABLE,
	.intr_type = GPIO_INTR_NEGEDGE,
	.pin_bit_mask = 1ULL << ITS_PIN_I2C_INT
};

static gpio_config_t init_pin_time = {
	.mode = GPIO_MODE_OUTPUT_OD,
	.pull_up_en = GPIO_PULLUP_ENABLE,
	.pull_down_en = GPIO_PULLDOWN_DISABLE,
	.intr_type = GPIO_INTR_DISABLE,
	.pin_bit_mask = 1ULL << ITS_PIN_TIME
};


void common_packet_to_route(uint8_t *data, uint16_t size);
uint8_t* common_imi_alloc(uint16_t size);

void init_basic(void) {
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	i2c_param_config(ITS_I2CTM_PORT, &init_pin_i2c_tm);
	i2c_driver_install(ITS_I2CTM_PORT, I2C_MODE_MASTER, 0, 0, 0);

	gpio_config(&init_pin_time);
	gpio_config(&init_pin_i2c_int);


}

void init_helper(void) {
	init_basic();
	gpio_install_isr_service(0);


	imi_init();
	imi_set_save_callback(common_packet_to_route);
	imi_set_alloc_callback(common_imi_alloc);
	imi_handler[0].address 	= ITS_ARK_1_ADDRESS;
	imi_handler[0].i2c_port = ITS_I2CTM_PORT;
	imi_handler[0].timeout  = 100;
	gpio_isr_handler_add(ITS_PIN_I2C_INT, imi_i2c_int_isr_handler, 0);


    wifi_init_sta();
    my_sntp_init();

}

uint8_t mv_packet[MAVLINK_MAX_PACKET_LEN];
uint8_t* common_imi_alloc(uint16_t size) {
	if (size > MAVLINK_MAX_PACKET_LEN) {
		return 0;
	}
	return mv_packet;
}
void common_packet_to_route(uint8_t *data, uint16_t size) {
	mavlink_message_t msg;
	mavlink_status_t mst;
	for (int i = 0; i < size - 1; i++) {
		mavlink_parse_char(MAVLINK_COMM_0, data[i], &msg, &mst);
	}
	if (mavlink_parse_char(MAVLINK_COMM_0, data[size - 1], &msg, &mst)) {
		its_rt_sender_ctx_t ctx;
		ctx.from_isr = 0;
		its_rt_route(&ctx, &msg, portMAX_DELAY);
	}
}

