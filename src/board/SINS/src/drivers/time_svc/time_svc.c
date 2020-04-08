/*
 * time_svc.c
 *
 *  Created on: 7 апр. 2020 г.
 *      Author: snork
 */

#include "time_svc.h"

#include <stm32f4xx_hal.h>
#include <assert.h>
#include <errno.h>

#include "sins_config.h"

#include "rtc.h"
#include "timers.h"
#include "time_util.h"


//! Флаг для взаимодействия прерывания RTC и осоновного треда
static volatile int _time_svc_started = 0;


// Прерывание на RTC аларм для запуска таймеров службы времени
void RTC_Alarm_IRQHandler()
{
	// Немедленно пускаем таймеры!
	time_svc_timers_start();

	// Выключаем прерывание
	__HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
	_time_svc_started = 1;
}



static void _disable_alarm_irq()
{
	// глушим прерывания от RTC
	__HAL_RTC_ALARM_DISABLE_IT(&hrtc, RTC_IT_ALRA);
	HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
}



int time_svc_init(void)
{
	int rc;

	// готовим RTC. По-хардкору или без. Нам нужно чтобы оно работало
	rc = time_svc_rtc_init();
	if (0 != rc)
		return rc;

	// взводим таймеры
	rc = time_svc_timers_prepare();
	if (0 != rc)
		return rc;

	// загружаем время из rtc
	struct tm tm;
	rc = time_svc_rtc_load(&tm);
	if (0 != rc)
		return rc;

	// Добавим две секунды
	// выставим это время в таймеры службы времени
	// и поставим на будильник в RTC
	// По прерыванию будильника RTC пуляем таймеры

	// Будем изящны
	const time_t next_tt = mktime(&tm) + 2;
	const struct tm * next_tm = gmtime(&next_tt);

	// Загружаем это время в таймеры
	time_svc_timers_initial_time_preload(next_tt);
	// Загружаем это время в будильник А RTC
	rc = time_svc_rtc_alarm_setup(next_tm, RTC_ALARM_A);
	if (0 != rc)
		return rc;


	// Мы еще не запустились
	_time_svc_started = 0;

	// Разрешаем прерывания для таймера А
	__HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRA);
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, ITS_SINS_TIME_SVC_ALARM_IRQ_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);


	uint32_t start_tick = HAL_GetTick();
	// Ждем пока служба времени запустится
	while(0 == _time_svc_started)
	{
		// Если мы тут крутимся больше трех секунд - очевидно что-то не то
		if (HAL_GetTick() - start_tick <= 3000)
		{
			// глушим прерывания от RTC
			_disable_alarm_irq();
			return -ETIMEDOUT;
		}
	}
	// Ок! Мы запустились!

	// глушим прерывания от RTC
	_disable_alarm_irq();
	return 0;
}
