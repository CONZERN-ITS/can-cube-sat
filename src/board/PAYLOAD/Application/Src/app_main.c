/*
 * app_main.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */


#include "app_main.h"

#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include <stm32f1xx_hal.h>

#include <its-i2c-link.h>

#include "main.h"

#include "mavlink_main.h"
#include "util.h"
#include "time_svc.h"

#include "sensors/analog.h"
#include "sensors/bme.h"
#include "sensors/me2o2.h"
#include "sensors/mics6814.h"
#include "sensors/integrated.h"


its_pld_status_t pld_g_status = {0};


typedef int (*initializer_t)(void);


extern I2C_HandleTypeDef hi2c1;

static int _init_sensors(bool force);
static bool _init_one_sensor(bool force, int32_t * init_error, initializer_t initilizer);

static void _process_bme_message(const mavlink_pld_bme280_data_t * msg);
static void _process_me2o2_message(mavlink_pld_me2o2_data_t * msg);
static void _process_mics_message(mavlink_pld_mics_6814_data_t * msg);
static void _process_owntemp_message(mavlink_own_temp_t * msg);

// Скидывать сообщения в текстовом виде в консольку для отладки
#define PROCESS_TO_PRINTF
// Скидывать сообщения в its_link
#define PROCESS_TO_ITSLINK

int app_main()
{
	int rc;
	int64_t teak = 0;

	time_svc_init();
	its_i2c_link_start(&hi2c1);
	_init_sensors(true);

	while(1)
	{
		uint32_t teak_start = HAL_GetTick();
		// Повторная попытка на инициализацию сенсоров, которые еще не
		_init_sensors(false);

		// Проверяем входящие пакеты
		mavlink_message_t input_msg;
		rc = mavlink_main_get_packet(&input_msg);
		if (0 == rc)
		{
			// Обрабытываем. Пока только для службы времени
			time_svc_on_mav_message(&input_msg);
		}

		if (0 == teak % 10)
		{
			mavlink_pld_bme280_data_t bme_msg = {0};
			rc = its_pld_bme280_read(&bme_msg);
			pld_g_status.bme_last_error = rc;
			pld_g_status.bme_error_counter += (0 == rc) ? 0 : 1;
			_process_bme_message(&bme_msg);
		}

		if (0 == teak % 10)
		{
			mavlink_pld_me2o2_data_t me2o2_msg = {0};
			rc = me2o2_read(&me2o2_msg);
			pld_g_status.me2o2_last_error = rc;
			pld_g_status.me2o2_error_counter += (0 == rc) ? 0 : 1;
			_process_me2o2_message(&me2o2_msg);
		}

		if (0 == teak % 10)
		{
			mavlink_pld_mics_6814_data_t mics_msg = {0};
			rc = mics6814_read(&mics_msg);
			pld_g_status.mics6814_last_error = rc;
			pld_g_status.mics6814_error_counter += (0 == rc) ? 0 : 1;
			_process_mics_message(&mics_msg);
		}

		if (0 == teak % 10)
		{
			mavlink_own_temp_t own_temp_msg;
			rc = its_pld_inttemp_read(&own_temp_msg);
			pld_g_status.integrated_last_error = rc;
			pld_g_status.integrated_error_counter += (0 == rc) ? 0 : 1;
			_process_owntemp_message(&own_temp_msg);
		}

		// Ждем начала следующего такта
		uint32_t next_teak_start = teak_start + 100;
		while (HAL_GetTick() < next_teak_start)
		{
			volatile int x = 0;
			(void)x;
		}
		teak++;
	}

	return 0;
}


static void _process_bme_message(const mavlink_pld_bme280_data_t * msg)
{
#ifdef PROCESS_TO_PRINTF
	printf("bme: t=%fC, p=%fpa, hum=%f%%, alt=%fm\n",
			msg->temperature, msg->pressure, msg->humidity, msg->altitude
	);

	printf("time = 0x%08"PRIX32"%08"PRIX32", %08"PRIX32"\n",
			(uint32_t)(msg->time_s >> 4*8), (uint32_t)(msg->time_s & 0xFFFFFFFF), msg->time_us
	);
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_pld_bme280_data_send_struct(MAVLINK_COMM_0, msg);
#endif

}


static void _process_me2o2_message(mavlink_pld_me2o2_data_t * msg)
{
#ifdef PROCESS_TO_PRINTF
	printf("me2o2: o2=%f%%\n",
			msg->o2_conc
	);

	printf("time = 0x%08"PRIX32"%08"PRIX32", %08"PRIX32"\n",
			(uint32_t)(msg->time_s >> 4*8), (uint32_t)(msg->time_s & 0xFFFFFFFF), msg->time_us
	);
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_pld_me2o2_data_send_struct(MAVLINK_COMM_0, msg);
#endif
}


static void _process_mics_message(mavlink_pld_mics_6814_data_t * msg)
{
#ifdef PROCESS_TO_PRINTF
	printf("mics6814: co=%fppm, nh3=%fppm, no2=%fppm\n",
			msg->co_conc, msg->nh3_conc, msg->no2_conc
	);

	printf("time = 0x%08"PRIX32"%08"PRIX32", %08"PRIX32"\n",
			(uint32_t)(msg->time_s >> 4*8), (uint32_t)(msg->time_s & 0xFFFFFFFF), msg->time_us
	);
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_pld_mics_6814_data_send_struct(MAVLINK_COMM_0, msg);
#endif
}


static void _process_owntemp_message(mavlink_own_temp_t * msg)
{
#ifdef PROCESS_TO_PRINTF
	printf("otemp: %fC\n",
			msg->deg
	);

	printf("time = 0x%08"PRIX32"%08"PRIX32", %08"PRIX32"\n",
			(uint32_t)(msg->time_s >> 4*8), (uint32_t)(msg->time_s & 0xFFFFFFFF), msg->time_us
	);
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_own_temp_send_struct(MAVLINK_COMM_0, msg);
#endif
}



static bool _init_one_sensor(bool force, int32_t * init_error, initializer_t initilizer)
{
	bool rc = true;
	if (force || *init_error != 0)
	{
		*init_error = initilizer();
		if (*init_error)
			rc = false;
	}

	return rc;
}


static int _init_sensors(bool force)
{
	bool rc = true;

	rc = rc && _init_one_sensor(force, &pld_g_status.bme_init_error, its_pld_bme280_init);
	rc = rc && _init_one_sensor(force, &pld_g_status.adc_init_error, its_pld_analog_init);
	rc = rc && _init_one_sensor(force, &pld_g_status.me2o2_init_error, me2o2_init);
	rc = rc && _init_one_sensor(force, &pld_g_status.mics6815_init_error, mics6814_init);
	rc = rc && _init_one_sensor(force, &pld_g_status.integrated_init_error, its_pld_inttemp_init);

	return rc;
}
