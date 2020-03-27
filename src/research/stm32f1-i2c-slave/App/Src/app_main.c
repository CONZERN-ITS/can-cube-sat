#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <stm32f1xx_hal.h>
#include <its-i2c-link.h>

#include "main.h"
#include "time.h"



int app_main()
{
	its_i2c_link_start();
	int i = 0;
	printf("hello stm32\n");
	while(1)
	{
		printf("teak = %d\n", i++);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		uint8_t packet_data[I2C_LINK_PACKET_SIZE] = {0};
		int rc = its_i2c_link_read(packet_data, I2C_LINK_PACKET_SIZE);
		printf("i2c_link_read rc = %d\n", rc);
		if (rc > 0) {
		    printf("!!!Packet. Size = %d. Message: %s\n", rc, packet_data);
		}

		char str[] = "Hello ESP32!";
		rc = its_i2c_link_write((uint8_t*) str, sizeof(str));
        printf("i2c_link_write rc = %d\n", rc);

		its_i2c_link_stats_t stats;
		its_i2c_link_stats(&stats);

		printf(
			"rx_done_cnt: %"PRIu16", rx_dropped_cnt: %"PRIu16", rx_errors_cnt: %"PRIu16"\n"
			"tx_done_cnt: %"PRIu16", tx_zeroes_cnt: %"PRIu16", tx_errors_cnt: %"PRIu16"\n"
			"listen_done_cnt: %"PRIu16", last_error: %"PRIu32", restarts: %"PRIu16"\n"
			,
			stats.rx_done_cnt, stats.rx_dropped_cnt, stats.rx_error_cnt,
			stats.tx_done_cnt, stats.tx_zeroes_cnt, stats.tx_error_cnt,
			stats.listen_done_cnt, stats.last_error, stats.restarts_cnt
		);

		HAL_Delay(100);
	}

	return 0;
}


