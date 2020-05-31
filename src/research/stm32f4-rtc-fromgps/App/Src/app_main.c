#include "main.h"

#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "time_util.h"
#include "ubx_stream_parser.h"
#include "gps.h"

typedef enum rtc_configuration_state_t
{
	// Ничего не запущено, ничего не готово
	RTC_CFG_STATE_IDLE,
	// RTC доступен для записи и остановлен. Ждем пакета от GPS, чтобы настроить время
	RTC_CFG_STATE_WAIT_PACKET,
	//! ждем PPS сигнала
	RTC_CFG_STATE_PACKET_READY,
	//! Часы запущены
	RTC_CFG_STATE_STARTED,
	//! Часы защищены от записи
	RTC_CFG_STATE_COMPLETE,
} rtc_configuration_state_t;


static int rtc_cfg_state = RTC_CFG_STATE_IDLE;


static void on_pps()
{
	if (RTC_CFG_STATE_PACKET_READY == rtc_cfg_state)
	{
		LL_RTC_ExitInitMode(RTC);
		rtc_cfg_state = RTC_CFG_STATE_STARTED;
	}
}


static void on_ubx_packet(void * userarg, const ubx_any_packet_t * packet)
{
	(void)userarg;

	switch (packet->pid)
	{
	case UBX_PID_NAV_TIMEGPS:
		printf("got gps_time packet: week %d, ms: %lu\n",
				packet->packet.gpstime.week,
				packet->packet.gpstime.tow_ms
		);

		{
			const ubx_gpstime_packet_t * pkt = &packet->packet.gpstime;
			if (
					(RTC_CFG_STATE_WAIT_PACKET == rtc_cfg_state)
					&& (pkt->valid_flags & UBX_NAVGPSTIME_FLAGS__LEAPS_VALID)
					&& (pkt->valid_flags & UBX_NAVGPSTIME_FLAGS__TOW_VALID)
					&& (pkt->valid_flags & UBX_NAVGPSTIME_FLAGS__WEEK_VALID)
			){

				struct timeval tv;
				gps_time_to_unix_time(pkt->week, pkt->tow_ms, &tv);
				tv.tv_sec -= pkt->leaps; // Вычитаем липосекунды
				tv.tv_sec += 1; // Настраиваемся на следующий фронт PPS

				struct tm * tm = gmtime(&tv.tv_sec);
				int rc = rtc_store(RTC, tm);
				if (0 == rc)
				{
					rtc_cfg_state = RTC_CFG_STATE_PACKET_READY;
					on_pps(); // имитируем приход pps прямо тут
				}

				printf("rtc configured with code = %d\n", rc);
			}
		}
		break;

	case UBX_PID_TIM_TP:
		/*
		printf("got tim_tp packet: week %d, ms: %lu\n",
				packet->packet.timtp.week,
				packet->packet.timtp.tow_ms
		);
		*/
		break;


	default:
		/*
		printf("got packet! pid = 0x%04X\n", (int)packet->pid);
		*/
		break;
	}
}


int app_main()
{

	// для pritnf
	__HAL_UART_ENABLE(&huart1);

	// начинаем получать пакеты gps
	assert(0 == gps_init(on_ubx_packet, NULL));
	// настраиваем gps
	int rc = gps_configure();
	printf("rc %d", rc);


	// Загоняем rtc в режим конфигурации
	LL_RTC_DisableWriteProtection(RTC);
	LL_RTC_EnterInitMode(RTC);
	rtc_cfg_state = RTC_CFG_STATE_WAIT_PACKET;




	while(1)
	{
		int polled_bytes = gps_poll();
		(void)polled_bytes;
		// printf("gps polled with %d bytes\n", polled_bytes);

		if (RTC_CFG_STATE_STARTED == rtc_cfg_state)
		{
			LL_RTC_EnableWriteProtection(RTC);
			printf("rtc configuration complete\n");
			rtc_cfg_state = RTC_CFG_STATE_COMPLETE;
		}

		if (RTC_CFG_STATE_COMPLETE == rtc_cfg_state)
		{
			struct tm tm;
			rtc_load(RTC, &tm);
			char buffer[sizeof "2011-10-08T07:07:09Z"] = {0};
			strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
			printf("rtc now is %s\n", buffer);
		}

		HAL_Delay(200);
	}

	return 0;
}
