/*
 * hardcore_rtc_init.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#include "rtc.h"

#include <assert.h>
#include <errno.h>

#include "../common.h"
#include "sins_config.h"
#include "time_util.h"


RTC_HandleTypeDef hrtc;

//! Настройка клоков RTC
/*! Это нужно делать, только если вдруг обнаружится что наш бэкап домен нифига не настроен */
static int _init_rtc_clocks()
{
	HAL_StatusTypeDef hal_error;
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};

	// Влючаем LSI/LSE
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE; // Из всех осциляторов трогаем только LSI
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // PLL не трогаем
#if ITS_SINS_RTC_CLKSOURCE_LSI
	RCC_OscInitStruct.LSIState = RCC_LSI_ON; // Включаем LSI
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
#else
	RCC_OscInitStruct.LSEState = RCC_LSI_OFF;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON; // Включаем LSE
#endif

	hal_error = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	// Указываем LSI как входной такт для RTC
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
#if ITS_SINS_RTC_CLKSOURCE_LSI
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
#else
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#endif
	/*
	A caution to be taken when HAL_RCCEx_PeriphCLKConfig() is used to select RTC clock selection, in this case
	the Reset of Backup domain will be applied in order to modify the RTC Clock source as consequence all backup
	domain (RTC and RCC_BDCR register expect BKPSRAM) will be reset
	*/
	hal_error = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	// А еще эта штука забывает выключить доступ на запись к бекап домену. Сделаем это за нее
	HAL_PWR_DisableBkUpAccess();
	return 0;
}


//! Настройка RTC халовского хендла
static void _init_rtc_handle()
{
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
#if ITS_SINS_RTC_CLKSOURCE_LSI
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
#else
	hrtc.Init.AsynchPrediv = 127; // предполагается что на LSE сидит часовой кварц на 32767
	hrtc.Init.SynchPrediv = 255;  // Эти делители дадут нам 1 Герц для секунд
#endif
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
}


int time_svc_rtc_hardcore_init()
{
	HAL_StatusTypeDef hal_error;
	int rc;

	// Настриваем LSE и запитываем от него RTC
	rc = _init_rtc_clocks();
	if (0 != rc)
		return rc;

	// Настриваем наш хеднл
	_init_rtc_handle();

	// Канонично настраиваем RTC
	HAL_PWR_EnableBkUpAccess();
	hal_error = HAL_RTC_Init(&hrtc);
	HAL_PWR_DisableBkUpAccess();

	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	// nvic в hal_msp

	// Выставляем 1 января 2000ого года
	RTC_TimeTypeDef time;

	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	time.Hours = RTC_ByteToBcd2(0);
	time.Minutes = RTC_ByteToBcd2(0);
	time.SecondFraction = 0; // This field will be used only by HAL_RTC_GetTime function
	time.Seconds = RTC_ByteToBcd2(0);
	time.StoreOperation = RTC_STOREOPERATION_RESET;
	time.SubSeconds = RTC_ByteToBcd2(0);
	time.TimeFormat = RTC_HOURFORMAT12_AM; // для 24ти часового формата, который мы подразумеваем нужно так

	RTC_DateTypeDef date;
	date.Date = 1;
	date.Month = RTC_MONTH_JANUARY;
#if ITS_SINS_TIME_SVC_RTC_BASE_YEAR != 2000
#	error "ITS_SINS_TIME_SVC_RTC_BASE_YEAR is not equal to 2000. Reconsider code here"
#endif
	date.WeekDay = RTC_WEEKDAY_SATURDAY; // это была суббота
	date.Year = RTC_ByteToBcd2(0x00);    // Будем считать это 2000ным годом

	HAL_StatusTypeDef hal_date_error, hal_time_error;
	HAL_PWR_EnableBkUpAccess();
	hal_date_error = HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);
	hal_time_error = HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_PWR_DisableBkUpAccess();

	if (HAL_OK != hal_date_error)
		return sins_hal_status_to_errno(hal_date_error);

	if (HAL_OK != hal_time_error)
		return sins_hal_status_to_errno(hal_time_error);

	// Ну, типо оно тикает
	return 0;
}


int time_svc_rtc_simple_init()
{
	// к сожалению в ХАЛ-Е не предусмотрены интерфейсы для работы
	// с RTC, который уже настроен кем-то до нас.
	// Поэтому сделаем это все для него и сделаем вид что ХАЛ нормально настроился

	// Настриваем наш хеднл
	_init_rtc_handle();

	// Разрешаем доступ к RTC по соответсвующей APB или какой-нибудь там шине
	// Короче прокидываем себе мостик к RTC в бэкап домен
	// (кажется)
	HAL_RTC_MspInit(&hrtc);

	// nvic в hal_msp

	// Делаем вид что все настроилось как задумано
	hrtc.Lock = HAL_UNLOCKED;
	hrtc.State = HAL_RTC_STATE_READY;

	// Готово
	return 0;
}


int time_svc_rtc_init()
{

#if ITS_SINS_RTC_FORCERESET
	// Насильный сброс бекап домена
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_BACKUPRESET_FORCE();
	__HAL_RCC_BACKUPRESET_RELEASE();
	HAL_PWR_DisableBkUpAccess();

#endif
	if (RCC->BDCR & RCC_BDCR_RTCEN)
	{
		// Значит оно уже работает. Настраиваемся по-простому
		int rc = time_svc_rtc_simple_init();
		if (0 == rc)
			return rc;
	}
	else
	{
		return -ENOSYS;
	}
	return 0;
}


int time_svc_rtc_load(struct tm * tm)
{
	int rc;
	HAL_StatusTypeDef hrc_date, hrc_time;

	// Сперва все вчитаем а потом будет разбираться с ошибками
	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;
	hrc_date = HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BCD);
	hrc_time = HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BCD);

	rc = sins_hal_status_to_errno(hrc_date);
	if (0 != rc)
		return rc;

	rc = sins_hal_status_to_errno(hrc_time);
	if (0 != rc)
		return rc;

	// Пересчитываем в struct tm
	rc = time_svc_rtc_to_struct_tm(&rtc_date, &rtc_time, tm);
	if (0 != rc)
		return rc;

	return 0;
}


int time_svc_rtc_store(const struct tm * tm)
{
	int rc;
	HAL_StatusTypeDef hrc_date, hrc_time;

	// Кастуем в халовские структуры
	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;

	rc = time_svc_struct_tm_to_rtc(tm, &rtc_date, &rtc_time);
	if (0 != rc)
		return rc;

	// зашиваем!
	HAL_PWR_EnableBkUpAccess();
	hrc_date = HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BCD);
	hrc_time = HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BCD);
	HAL_PWR_DisableBkUpAccess();

	// Проверяем как зашилось
	rc = sins_hal_status_to_errno(hrc_date);
	if (0 != rc)
		return rc;

	rc = sins_hal_status_to_errno(hrc_time);
	if (0 != rc)
		return rc;

	return 0;
}


int time_svc_rtc_alarm_setup(const struct tm * tm, uint32_t alarm)
{
	int rc;
	HAL_StatusTypeDef hrc;

	// Кастуем в халовские структуры
	RTC_AlarmTypeDef rtc_alarm;
	RTC_DateTypeDef rtc_date;

	rc = time_svc_struct_tm_to_rtc(tm, &rtc_date, &rtc_alarm.AlarmTime);
	if (0 != rc)
		return rc;

	rtc_alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY; //RTC_ALARMMASK_NONE; 	// Алармим по всем параметрам даты
	rtc_alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL; // Забиваем на сабсекунды
	rtc_alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE; // Работаем по дате а не еженедельно
	rtc_alarm.AlarmDateWeekDay = rtc_date.Date; // Уточняем собственно дату
	rtc_alarm.Alarm = alarm;

	// Выставляем
	HAL_PWR_EnableBkUpAccess();
	hrc = HAL_RTC_SetAlarm(&hrtc, &rtc_alarm, RTC_FORMAT_BCD);
	HAL_PWR_DisableBkUpAccess();

	rc = sins_hal_status_to_errno(hrc);
	if (0 != rc)
		return rc;

	return 0;
}

