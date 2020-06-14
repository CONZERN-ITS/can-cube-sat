/*
 * app_main.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include <stm32f1xx_hal.h>

#include <its-i2c-link.h>

#include "main.h"
#include "led.h"

#include "mavlink_main.h"
#include "util.h"
#include "time_svc.h"

#include "sensors/analog.h"
#include "sensors/bme.h"
#include "sensors/me2o2.h"
#include "sensors/mics6814.h"
#include "sensors/integrated.h"

typedef struct its_pld_status_t
{
	int32_t bme_last_error;
	uint16_t bme_error_counter;

	int32_t adc_last_error;
	uint16_t adc_error_counter;

	uint16_t restarts_count;
} its_pld_status_t;


static its_pld_status_t _status = {0};


extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;

typedef int (*initializer_t)(void);

//! Собираем собственную статистику в мав пакет
static void _collect_own_stats(mavlink_pld_stats_t * msg);
//! Пакуем i2c-link статистику в мав пакет
static void _collect_i2c_link_stats(mavlink_i2c_link_stats_t * msg);

//! Анализ кода возвращенного bme операцией
static int _bme_op_analysis(int rc);
//! Убеждаемся в том, что БМЕ функционален. Пытаемся его рестартнуть если нет
static int _bme_restart_if_need_so(void);

//! Анализ кода возвращенного аналоговой операцией
static int _analog_op_analysis(int rc);
//! Убеждаемся в том, что АЦП функционален. Пытаемся его рестартнуть если нет
static int _analog_restart_if_need_so(void);

//! Обработка вхоядщие пакетов
static void _process_input_packets(void);

// TODO:
/* tx_overrun в i2c-link-stats
   ассерты и ErrorHandler-ы хала адекватно работают
   калибрануть все и вся
   убедиться в том, что счетчики в мавлинк пакетах работают правильно
   вставить ассерты в error-handler-ы
   настроить частоты пакетов
   проверить настройки i2c1 (который i2c-link)
   проверить как работает рестарт АЦП
   вставить задержку после bme280_soft_reset - иначе первые данные получаются кривые
 */

int app_main()
{
	// Грузим из бэкап регистров количество рестартов, которое с нами случилось
	_status.restarts_count = LL_RTC_BKP_GetRegister(BKP, LL_RTC_BKP_DR1);
	_status.restarts_count += 1;
	LL_RTC_BKP_SetRegister(BKP, LL_RTC_BKP_DR1, _status.restarts_count);

	// Включаем все
	led_init();
	time_svc_init();
	its_i2c_link_start(&hi2c1);

	_bme_op_analysis(bme_init());
	_analog_op_analysis(analog_init());
	mics6814_init();

	// После перезагрузки будем аж пол секунды светить лампочкой
	uint32_t tick_begin = HAL_GetTick();
	uint32_t tick_led_unlock = tick_begin + 1000;

	led_set(true);

	int64_t tock = 0;
	while(1)
	{
		// Сбрасываем вотчдог
		HAL_IWDG_Refresh(&hiwdg);

		// Запоминаем когда этот такт начался
		uint32_t tock_start_tick = HAL_GetTick();
		// Планируем начало следующего
		uint32_t next_tock_start_tick = tock_start_tick + 200;

		// В начале такта включаем диод (если после загрузки прошло нужное время)
		// И запоминаем не раньше какого времени нужно выключить светодиод
		uint32_t led_off_tick;
		if (HAL_GetTick() >= tick_led_unlock)
		{
			led_set(true);
			led_off_tick = tock_start_tick + 10;
		}
		else
		{
			// Если еще не пора - то и выключать не будем диод в конце такта
			led_off_tick = tick_led_unlock;
		}

		// Проверяем входящие пакеты
		_process_input_packets();

		if (0 == tock % 10)
		{
			if (0 == _bme_restart_if_need_so())
			{
				mavlink_pld_bme280_data_t bme_msg = {0};
				int rc = _bme_op_analysis(bme_read(&bme_msg));
				if (0 == rc)
					mav_main_process_bme_message(&bme_msg);
			}
		}


		if (0 == tock % 10)
		{
			if (0 == _analog_restart_if_need_so())
			{
				mavlink_pld_me2o2_data_t me2o2_msg = {0};
				int rc = _analog_op_analysis(me2o2_read(&me2o2_msg));
				if (0 == rc)
					mav_main_process_me2o2_message(&me2o2_msg);
			}
		}


		if (0 == tock % 10)
		{
			if (0 == _analog_restart_if_need_so())
			{
				mavlink_pld_mics_6814_data_t mics_msg = {0};
				if (0 == _analog_op_analysis(mics6814_read(&mics_msg)))
					mav_main_process_mics_message(&mics_msg);
			}
		}


		if (0 == tock % 10)
		{
			if (0 == _analog_restart_if_need_so())
			{
				mavlink_own_temp_t own_temp_msg;
				if (0 == _analog_op_analysis(integrated_read(&own_temp_msg)))
					mav_main_process_owntemp_message(&own_temp_msg);
			}
		}


		if (0 == tock % 20)
		{
			mavlink_pld_stats_t pld_stats_msg;
			_collect_own_stats(&pld_stats_msg);
			mav_main_process_own_stats(&pld_stats_msg);
		}


		if (0 == tock % 30)
		{
			mavlink_i2c_link_stats_t i2c_stats_msg;
			_collect_i2c_link_stats(&i2c_stats_msg);
			mav_main_process_i2c_link_stats(&i2c_stats_msg);
		}


		// Ждем начала следующего такта
		uint32_t now;
		bool led_done  = false;
		do {
			now = HAL_GetTick();
			// Выключаем диод если пора
			if (!led_done && now >= led_off_tick)
			{
				led_set(false);
				led_done  = true;
			}

		} while(now < next_tock_start_tick);
		// К следующему такту
		tock++;
	}

	return 0;
}


static void _collect_own_stats(mavlink_pld_stats_t * msg)
{
	struct timeval tmv;
	time_svc_gettimeofday(&tmv);
	msg->time_s = tmv.tv_sec;
	msg->time_us = tmv.tv_usec;
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


static void _process_input_packets()
{
	mavlink_message_t input_msg;
	int rc = mav_main_get_packet(&input_msg);
	if (0 == rc)
	{
		// Обрабытываем. Пока только для службы времени
		time_svc_on_mav_message(&input_msg);
	}
}


static int _bme_op_analysis(int rc)
{
	_status.bme_last_error = rc;
	if (rc)
		_status.bme_error_counter++;

	return _status.bme_last_error;
}


static int _bme_restart_if_need_so(void)
{
	if (_status.bme_last_error)
		_bme_op_analysis(bme_restart());

	return _status.bme_last_error;
}


static int _analog_op_analysis(int rc)
{
	_status.adc_last_error = rc;
	if (rc)
		_status.adc_error_counter++;

	return _status.adc_last_error;
}


static int _analog_restart_if_need_so(void)
{
	if (_status.adc_last_error)
		_analog_op_analysis(analog_restart());

	return _status.adc_last_error;
}
