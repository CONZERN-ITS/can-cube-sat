#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <stm32f1xx_hal.h>
#include <its-i2c-link.h>
#include <its-time.h>

#include "main.h"
#include "sys/time.h"
#include "../../Drivers/mavlink/its/mavlink.h"




int app_main()
{
	its_i2c_link_start();
	int i = 0;
	printf("hello stm32\n");


	its_time_init();
	its_time_t ts = {0};
	its_settimeofday(&ts);

	uint32_t prev_print = HAL_GetTick();
	while (1)
	{
	    uint32_t now = HAL_GetTick();
	    //printf("%d\n", sizeof(clock_t));
		//printf("teak = %d\n", i++);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		uint8_t packet_data[I2C_LINK_PACKET_SIZE] = {0};
		int rc = its_i2c_link_read(packet_data, I2C_LINK_PACKET_SIZE);

		if (rc) {
		    mavlink_message_t msg = {0};
		    mavlink_status_t mst = {0};
		    for (int i = 0; i < rc - 1; i++) {
		        mavlink_parse_char(MAVLINK_COMM_0, packet_data[i], &msg, &mst);
		    }
		    if (mavlink_parse_char(MAVLINK_COMM_0, packet_data[rc - 1], &msg, &mst)) {
		        switch(msg.msgid) {
		        case MAVLINK_MSG_ID_TIMESTAMP: {
		            mavlink_timestamp_t mts;
		            mavlink_msg_timestamp_decode(&msg, &mts);
		            its_time_t t;
                    t.sec = mts.time_s;
                    t.usec = mts.time_us / 1000;
		            its_sync_time(&t);
		        }
		        }
		    }
		}

        //printf("i2c_link_write rc = %d\n", rc);

		its_i2c_link_stats_t stats;
		its_i2c_link_stats(&stats);

		/*
		printf(
			"rx_done_cnt: %"PRIu16", rx_dropped_cnt: %"PRIu16", rx_errors_cnt: %"PRIu16"\n"
			"tx_done_cnt: %"PRIu16", tx_zeroes_cnt: %"PRIu16", tx_errors_cnt: %"PRIu16"\n"
			"listen_done_cnt: %"PRIu16", last_error: %"PRIu32", restarts: %"PRIu16"\n"
			,
			stats.rx_done_cnt, stats.rx_dropped_cnt, stats.rx_error_cnt,
			stats.tx_done_cnt, stats.tx_zeroes_cnt, stats.tx_error_cnt,
			stats.listen_done_cnt, stats.last_error, stats.restarts_cnt
		);*/
		if (now - prev_print > 1000) {
	        its_time_t time;
	        its_gettimeofday(&time);
	        printf("TIME: %lu:%03hu\n", (uint32_t) time.sec, time.usec);
	        prev_print += 1000;

	        int magic = 42; //FIXME: Надо добавить системы
	        mavlink_message_t msg;
	        mavlink_timestamp_t mts = {0};
            mts.time_s = time.sec;
            mts.time_us = time.usec * 1000;
	        mavlink_msg_timestamp_encode(magic, magic, &msg, &mts);

	        uint8_t buf[MAVLINK_MAX_PACKET_LEN];
	        uint16_t size = mavlink_msg_to_send_buffer(buf, &msg);
	        rc = its_i2c_link_write(buf, size);

		}


		HAL_Delay(100);
	}

	return 0;
}


