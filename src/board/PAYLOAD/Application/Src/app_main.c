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

extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;

// Скидывать сообщения в текстовом виде в консольку для отладки
#define PROCESS_TO_PRINTF
// Скидывать сообщения в its_link
#define PROCESS_TO_ITSLINK


typedef int (*initializer_t)(void);

static bool _init_one_sensor(bool force, int32_t * init_error, initializer_t initilizer);
static int _init_sensors(bool force);
static void _collect_own_stats(mavlink_pld_stats_t * msg);
static void _collect_i2c_link_stats(mavlink_i2c_link_stats_t * msg);

static void _process_bme_message(const mavlink_pld_bme280_data_t * msg);
static void _process_me2o2_message(mavlink_pld_me2o2_data_t * msg);
static void _process_mics_message(mavlink_pld_mics_6814_data_t * msg);
static void _process_owntemp_message(mavlink_own_temp_t * msg);
static void _process_own_stats(mavlink_pld_stats_t * msg);
static void _process_i2c_link_stats(mavlink_i2c_link_stats_t * msg);

// TODO:
/* tx_overrun в i2c-link-stats
   i2c reset и полная реинициализация по ошибке bme280
   ассерты и ErrorHandler-ы хала адекватно работают
   калибрануть все и вся
   убедиться в том, что счетчики в мавлинк пакетах работают правильно
   не отправлять пакеты при ошибках
   вставить ассерты в error-handler-ы
   настроить частоты пакетов

 */

int app_main()
{
	int rc;
	int64_t teak = 0;

	time_svc_init();
	its_i2c_link_start(&hi2c1);
	_init_sensors(true);

	while(1)
	{
		// Сбрасываем вотчдог
		HAL_IWDG_Refresh(&hiwdg);

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

		if (0 == teak % 20)
		{
			mavlink_pld_stats_t pld_stats_msg;
			_collect_own_stats(&pld_stats_msg);
			_process_own_stats(&pld_stats_msg);
		}

		if (0 == teak % 30)
		{
			mavlink_i2c_link_stats_t i2c_stats_msg;
			_collect_i2c_link_stats(&i2c_stats_msg);
			_process_i2c_link_stats(&i2c_stats_msg);
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
	rc = rc && _init_one_sensor(force, &pld_g_status.mics6814_init_error, mics6814_init);
	rc = rc && _init_one_sensor(force, &pld_g_status.integrated_init_error, its_pld_inttemp_init);

	return rc;
}


static void _collect_own_stats(mavlink_pld_stats_t * msg)
{
	struct timeval tmv;
	time_svc_gettimeofday(&tmv);
	msg->time_s = tmv.tv_sec;
	msg->time_us = tmv.tv_usec;

	msg->bme_init_error = pld_g_status.bme_init_error;
	msg->bme_last_error = pld_g_status.bme_last_error;
	msg->bme_error_counter = pld_g_status.bme_error_counter;

	msg->adc_init_error = pld_g_status.adc_init_error;
	msg->adc_last_error = pld_g_status.adc_last_error;
	msg->adc_error_counter = pld_g_status.adc_error_counter;

	msg->me2o2_init_error = pld_g_status.me2o2_init_error;
	msg->me2o2_last_error = pld_g_status.me2o2_last_error;
	msg->me2o2_error_counter = pld_g_status.me2o2_error_counter;

	msg->mics6814_init_error = pld_g_status.mics6814_init_error;
	msg->mics6814_last_error = pld_g_status.mics6814_last_error;
	msg->mics6814_error_counter = pld_g_status.mics6814_error_counter;

	msg->integrated_init_error = pld_g_status.integrated_init_error;
	msg->integrated_last_error = pld_g_status.integrated_last_error;
	msg->integrated_error_counter = pld_g_status.integrated_error_counter;
}


static void _collect_i2c_link_stats(mavlink_i2c_link_stats_t * msg)
{
	struct timeval tmv;
	time_svc_gettimeofday(&tmv);
	msg->time_s = tmv.tv_sec;
	msg->time_us = tmv.tv_usec;

	its_i2c_link_stats_t statsbuf;
	its_i2c_link_stats(&statsbuf);

	msg->rx_done_cnt = statsbuf.rx_done_cnt;
	msg->rx_dropped_cnt = statsbuf.rx_dropped_cnt;
	msg->rx_error_cnt = statsbuf.rx_error_cnt;
	msg->tx_done_cnt = statsbuf.tx_done_cnt;
	msg->tx_zeroes_cnt = statsbuf.tx_zeroes_cnt;
	msg->tx_error_cnt = statsbuf.tx_error_cnt;
	msg->restarts_cnt = statsbuf.restarts_cnt;
	msg->listen_done_cnt = statsbuf.listen_done_cnt;
	msg->last_error = statsbuf.last_error;
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

	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
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

	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
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

	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
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

	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_own_temp_send_struct(MAVLINK_COMM_0, msg);
#endif
}


static void _process_own_stats(mavlink_pld_stats_t * msg)
{
#ifdef PROCESS_TO_PRINTF
	printf("bme-> ie: %"PRIi32", le: %"PRIi32", ec: %"PRIu16":\n",
			msg->bme_init_error, msg->bme_last_error, msg->bme_error_counter
	);

	printf("adc-> ie: %"PRIi32", le: %"PRIi32", ec: %"PRIu16":\n",
			msg->adc_init_error, msg->adc_last_error, msg->adc_error_counter
	);

	printf("me2o2-> ie: %"PRIi32", le: %"PRIi32", ec: %"PRIu16":\n",
			msg->me2o2_init_error, msg->me2o2_last_error, msg->me2o2_error_counter
	);

	printf("mics6814-> ie: %"PRIi32", le: %"PRIi32", ec: %"PRIu16":\n",
			msg->mics6814_init_error, msg->mics6814_last_error, msg->mics6814_error_counter
	);

	printf("integrated-> ie: %"PRIi32", le: %"PRIi32", ec: %"PRIu16":\n",
			msg->integrated_init_error, msg->integrated_last_error, msg->integrated_error_counter
	);

	printf("time = 0x%08"PRIX32"%08"PRIX32", %08"PRIX32"\n",
			(uint32_t)(msg->time_s >> 4*8), (uint32_t)(msg->time_s & 0xFFFFFFFF), msg->time_us
	);
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_pld_stats_send_struct(MAVLINK_COMM_0, msg);
#endif
}


static void _process_i2c_link_stats(mavlink_i2c_link_stats_t * msg)
{
#ifdef PROCESS_TO_PRINTF
	printf("it2link rx-> done: %"PRIu16", dropped: %"PRIu16", errors: %"PRIu16":\n",
			msg->rx_done_cnt, msg->rx_dropped_cnt, msg->rx_error_cnt
	);

	printf("it2link tx-> done: %"PRIu16", dropped: %"PRIu16", errors: %"PRIu16":\n",
			msg->tx_done_cnt, msg->tx_zeroes_cnt, msg->tx_error_cnt
	);

	printf("it2link restarts: %"PRIu16", listen done: %"PRIu16", last error: %"PRIi32":\n",
			msg->restarts_cnt, msg->listen_done_cnt, msg->last_error
	);

	printf("time = 0x%08"PRIX32"%08"PRIX32", %08"PRIX32"\n",
			(uint32_t)(msg->time_s >> 4*8), (uint32_t)(msg->time_s & 0xFFFFFFFF), msg->time_us
	);
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
#endif

#ifdef PROCESS_TO_ITSLINK
	mavlink_msg_i2c_link_stats_send_struct(MAVLINK_COMM_0, msg);
#endif
}
