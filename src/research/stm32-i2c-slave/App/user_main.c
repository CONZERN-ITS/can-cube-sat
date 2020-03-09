#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <stm32f1xx_hal.h>

#include "main.h"
#include "i2c-link.h"


int user_main()
{
	i2c_link_start();

	int i = 0;
	printf("hello stm32\n");
	while(1)
	{
		printf("teak = %d\n", i++);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		uint8_t packet_data[I2C_LINK_PACKET_SIZE] = {0};
		int rc = i2c_link_read(packet_data, I2C_LINK_PACKET_SIZE);
		printf("i2c_link_read rc = %d\n", rc);

		if (rc > 0)
		{
			rc = i2c_link_write(packet_data, I2C_LINK_PACKET_SIZE);
			printf("i2c_link_write rc = %d\n", rc);
		}

		i2c_link_stats_t stats;
		i2c_link_stats(&stats);

		printf(
			"rx_done_cnt: %u, rx_dropped_cnt: %u, rx_errors_cnt: %u\n"
			"tx_done_cnt: %u, tx_zeroes_cnt: %u, tx_errors_cnt: %u\n"
			"listen_done_cnt: %u, last_error: %u\n"
			,
			stats.rx_done_cnt, stats.rx_dropped_cnt, stats.rx_error_cnt,
			stats.tx_done_cnt, stats.tx_zeroes_cnt, stats.tx_zeroes_cnt,
			stats.listen_done_cnt, stats.last_error
		);

		HAL_Delay(500);
	}

	return 0;
}


