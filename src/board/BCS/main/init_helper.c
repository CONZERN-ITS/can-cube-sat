/*
 * pin_init.c
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */
/*
 * Данный модуль занимается вопросами иницализации всей системе
 * как низкоуровневыми, так и высокоуровневыми. Часть инициализаций
 * располагается в компонентах для очевидного удобства.
 */

#include "init_helper.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/uart.h"


#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_intr_alloc.h"

#include "internet.h"
#include "pinout_cfg.h"
#include "imi.h"
#include "uart_mavlink.h"
#include "time_sync.h"

#include "router.h"
#include "mavlink/its/mavlink.h"
#include "mavlink_help2.h"
#include "radio.h"
#include "sdio.h"
#include "control_heat.h"
#include "control_magnet.h"
#include "control_vcc.h"
#include "sensors.h"
#include "log_collector.h"
#include "input_op.h"
#include "operator_ip.h"

static i2c_config_t init_pin_i2c_tm  = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = ITS_PIN_I2CTM_SDA,
	.sda_pullup_en = GPIO_PULLUP_DISABLE,
	.scl_io_num = ITS_PIN_I2CTM_SCL,
	.scl_pullup_en = GPIO_PULLUP_DISABLE,
	.master.clk_speed = ITS_I2CTM_FREQ
};

static gpio_config_t init_pin_time = {
	.mode = GPIO_MODE_OUTPUT_OD,
	.pull_up_en = GPIO_PULLUP_ENABLE,
	.pull_down_en = GPIO_PULLDOWN_DISABLE,
	.intr_type = GPIO_INTR_DISABLE,
	.pin_bit_mask = 1ULL << ITS_PIN_TIME
};

static uart_config_t init_pin_uart = {
	.baud_rate = 115200,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	.source_clk = UART_SCLK_APB,
};
static QueueHandle_t quart;
static mavlink_channel_t i2c_chan;

static uart_config_t init_pin_uart0 = {
	.baud_rate = 115200,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	.source_clk = UART_SCLK_APB,
};

void common_packet_to_route(uint8_t *data, uint16_t size);
uint8_t* common_imi_alloc(uint16_t size);

static imi_config_t imi_config = {
	.i2c_port = ITS_I2CTM_PORT,
	.i2c_int = ITS_PIN_I2C_INT,
	.address_count = ITS_I2CTM_DEV_COUNT,
	.ticksToWaitForOne = 100 / portTICK_RATE_MS,
	.save = common_packet_to_route,
	.alloc = common_imi_alloc
};
#ifndef ITS_ESP_DEBUG
static spi_bus_config_t buscfg={
	.miso_io_num = ITS_PIN_SPISR_MISO,
	.mosi_io_num = ITS_PIN_SPISR_MOSI,
	.sclk_io_num = ITS_PIN_SPISR_SCK,
	.quadwp_io_num = -1, //not used
	.quadhd_io_num = -1, //not used
	.max_transfer_sz = ITS_BSK_COUNT * 5
};
shift_reg_handler_t hsr;

static op_ip_t hop;



static void task_led(void *arg) {
	gpio_config_t gc = {
			.mode = GPIO_MODE_OUTPUT_OD,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE,
			.pin_bit_mask = 1ULL << ITS_PIN_LED,
	};
	gpio_config(&gc);
	int x = 1;
	while (1) {
		int rc = gpio_set_level(ITS_PIN_LED, x);
		x ^= 1;
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}
#endif

void init_basic(void) {
#ifndef ITS_ESP_DEBUG
	xTaskCreatePinnedToCore(task_led, "Led", configMINIMAL_STACK_SIZE + 2000, 0, 1, 0, tskNO_AFFINITY);
#endif
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	i2c_param_config(ITS_I2CTM_PORT, &init_pin_i2c_tm);
	i2c_driver_install(ITS_I2CTM_PORT, I2C_MODE_MASTER, 0, 0, 0);

	//esp - stm32f4
	uart_param_config(ITS_UARTE_PORT, &init_pin_uart);
	uart_driver_install(ITS_UARTE_PORT, ITS_UARTE_RX_BUF_SIZE, ITS_UARTE_TX_BUF_SIZE, ITS_UARTE_QUEUE_SIZE, &quart, 0);
	uart_set_pin(ITS_UARTE_PORT, ITS_PIN_UARTE_TX, ITS_PIN_UARTE_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

	//esp - radio
	uart_param_config(ITS_UARTR_PORT, &init_pin_uart0);
	uart_driver_install(ITS_UARTR_PORT, ITS_UARTR_RX_BUF_SIZE, ITS_UARTR_TX_BUF_SIZE, 0, 0, 0);
	uart_set_pin(ITS_UARTR_PORT, ITS_PIN_UARTR_TX, ITS_PIN_UARTR_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

#ifndef ITS_ESP_DEBUG
	//shift reg
	ret=spi_bus_initialize(ITS_SPISR_PORT, &buscfg, 0);
	ESP_ERROR_CHECK(ret);
	ESP_LOGD("SYSTEM", "Bus inited");
#endif

	//time sync
	gpio_config(&init_pin_time);

	gpio_install_isr_service(0);
}

#ifndef ITS_ESP_DEBUG
static void test_task(void *arg) {
	int x = 1;
	shift_reg_load(&hsr);
	while (1) {
		vTaskDelay(5000 / portTICK_PERIOD_MS);/*
		for (int i = 0; i < hsr.arr_size; i++) {
			hsr.byte_arr[i] ^= 0xFF;
		}

		shift_reg_load(&hsr);*/
	}
}
#endif
void init_helper(void) {
	init_basic();

	//imi_init();

	//Связь со всеми уст-ми на imi
	i2c_chan = mavlink_claim_channel();
	imi_install(&imi_config, ITS_IMI_PORT);
	imi_add_address(ITS_IMI_PORT, ITS_ARK_ADDRESS);
	imi_add_address(ITS_IMI_PORT, ITS_PLD_ADDRESS);
	imi_start(ITS_IMI_PORT);

	//Связь с SINS
	uart_mavlink_install(ITS_UARTE_PORT, quart);
#ifndef ITS_ESP_DEBUG
	shift_reg_init_spi(&hsr, ITS_SPISR_PORT, ITS_BSK_COUNT * ITS_SR_PACK_SIZE, 100 / portTICK_PERIOD_MS, ITS_PIN_SPISR_SS);
	ESP_LOGD("SYSTEM", "Shift reg inited");

	control_vcc_init(&hsr, 0);
	control_vcc_bsk_enable(0, 1);
	control_vcc_bsk_enable(1, 1);
	control_vcc_bsk_enable(2, 1);
	control_vcc_bsk_enable(3, 1);
	control_vcc_bsk_enable(4, 1);
	control_vcc_bsk_enable(5, 1);
	control_magnet_init(&hsr, 2, 3);
	control_heat_init(&hsr, 1, 0);

	shift_reg_load(&hsr);
/*
	control_magnet_enable(ITS_BSK_1, 1);
	control_magnet_enable(ITS_BSK_2A, -1);
	control_magnet_enable(ITS_BSK_3, 1);
	control_magnet_enable(ITS_BSK_4, -1);
	control_magnet_enable(ITS_BSK_5, 1);
	control_magnet_enable(ITS_BSK_2, -1);


	control_heat_bsk_enable(ITS_BSK_1, 1);
	control_heat_bsk_enable(ITS_BSK_2, 1);
	control_heat_bsk_enable(ITS_BSK_2A, 1);
	control_heat_bsk_enable(ITS_BSK_3, 1);
	control_heat_bsk_enable(ITS_BSK_4, 1);
	control_heat_bsk_enable(ITS_BSK_5, 1);
//	hsr.byte_arr[0] = 0x99;
//	hsr.byte_arr[1] = 0xBB;
//	hsr.byte_arr[2] = 0xBB;
	shift_reg_load(&hsr);*/

	//xTaskCreate(test_task, "test task", configMINIMAL_STACK_SIZE + 2000, 0, 3, 0);
	sensors_init();
#endif

	ESP_LOGD("SYSTEM", "Start wifi init");
#if ITS_WIFI_SERVER
	wifi_init_ap();
	static ts_sync ts = {0};
	ts.pin = ITS_PIN_UARTE_INT;
	time_sync_from_sins_install(&ts);
	radio_send_init();
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	while (sd_init()) {
		ESP_LOGD("SYSTEM","Trying launch SD");
	}
#else
	wifi_init_sta();
	time_sync_from_bcs_install(&ITS_WIFI_SERVER_ADDRESS);
#endif


	ESP_LOGD("SYSTEM", "Wifi inited");
	log_collector_init(0);
	op_config_ip(&hop, 53597);
	op_init((op_handler_t *)&hop);
	input_init((op_handler_t *)&hop);

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
		mavlink_parse_char(i2c_chan, data[i], &msg, &mst);
	}
	if (mavlink_parse_char(i2c_chan, data[size - 1], &msg, &mst)) {
		its_rt_sender_ctx_t ctx;
		ctx.from_isr = 0;
		its_rt_route(&ctx, &msg, 100 / portTICK_RATE_MS);
	}
}

