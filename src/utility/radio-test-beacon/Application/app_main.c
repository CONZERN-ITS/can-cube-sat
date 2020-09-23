/*
 * app_main.c
 *
 *  Created on: Aug 15, 2020
 *      Author: snork
 */

#include <stm32f1xx_hal.h>

#include "gps/gps.h"

#include "downlink.h"
#include "led.h"

static int config_finished = 0;


static void do_blinks(int count, int delay)
{
	// Мигаем лампочкой три раза
	for (int i = 0 ; i < count; i++)
	{
		led_set(true);
		HAL_Delay(delay);
		led_set(false);
		HAL_Delay(delay);
	}
}

static void gps_packet_callback(void * user_arg, const ubx_any_packet_t * packet)
{
	if (UBX_PID_NAV_SOL == packet->pid)
	{
		if (config_finished)
		{
			downlink_gps_packet(packet);
			const ubx_navsol_packet_t * navsol_packet = (ubx_navsol_packet_t *)packet;
			switch (navsol_packet->gps_fix)
			{
			default:
				// Тут бывает time_fix_only например
			case UBX_FIX_TYPE__NO_FIX:
				do_blinks(1, 30);
				break;

			case UBX_FIX_TYPE__2D:
				do_blinks(2, 30);
				break;

			case UBX_FIX_TYPE__3D:
				do_blinks(3, 30);
				break;
			}
		}
	}
}


int app_main()
{
	downlink_init();
	led_init();
	gps_init(gps_packet_callback, NULL);
	int cfg_result = gps_configure();

	if (0 == cfg_result)
	{
		do_blinks(3, 300);
	}
	else
	{
		// Если не получилось - перезапускаемся
		do_blinks(5, 300);
		NVIC_SystemReset();
	}

	gps_flush();
	config_finished = 1;

	while(1)
	{
		gps_poll();
	}

	return 0;
}
