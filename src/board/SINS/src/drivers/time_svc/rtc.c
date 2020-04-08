/*
 * hardcore_rtc_init.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#include "rtc.h"

#include <assert.h>
#include <errno.h>

#include "sins_config.h"

RTC_HandleTypeDef hrtc;


//! Настройка клоков RTC
/*! Это нужно делать, только если вдруг обнаружится что наш бэкап домен нифига не настроен */
static void _init_rtc_clocks()
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};

	// Влючаем LSE
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE; // Из всех осциляторов трогаем только LSE
	RCC_OscInitStruct.LSEState = RCC_LSE_ON; // Включаем lSE
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // PLL не трогаем
	assert(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK);

	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	// Указываем LSE как входной такт для RTC
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	/*
	A caution to be taken when HAL_RCCEx_PeriphCLKConfig() is used to select RTC clock selection, in this case
	the Reset of Backup domain will be applied in order to modify the RTC Clock source as consequence all backup
	domain (RTC and RCC_BDCR register expect BKPSRAM) will be reset
	*/
	assert(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) == HAL_OK);

	// А еще эта штука забывает выключить доступ на запись к бекап домену. Сделаем это за нее
	HAL_PWR_DisableBkUpAccess();
}


//! Настройка RTC халовского хендла
static void _init_rtc_handle()
{
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127; // предполагается что на LSE сидит часовой кварц на 32767
	hrtc.Init.SynchPrediv = 255;  // Эти делители дадут нам 1 Герц для секунд
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
}


void time_svc_rtc_hardcore_init()
{
	// Настриваем LSE и запитываем от него RTC
	_init_rtc_clocks();

	// Настриваем наш хеднл
	_init_rtc_handle();

	// Канонично настраиваем RTC
	HAL_RTC_Init(&hrtc);

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
	date.WeekDay = RTC_WEEKDAY_SATURDAY; // это была суббота
	date.Year = RTC_ByteToBcd2(0x00);    // Будем считать это 2000ным годом

	HAL_PWR_EnableBkUpAccess();
	HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);
	HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_PWR_DisableBkUpAccess();

	// Ну, типо оно тикает
}


void time_svc_rtc_simple_init()
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

	// Делаем вид что все настроилось как задумано
	hrtc.Lock = HAL_UNLOCKED;
	hrtc.State = HAL_RTC_STATE_READY;

	// Готово
}


void time_scv_rtc_init(void)
{
	if (RCC->BDCR & RCC_BDCR_RTCEN)
	{
		// Значит оно уже работает. Настраиваемся по-простому
		time_svc_rtc_simple_init();
	}
	else
	{
		// Значит оно не работает. Запускаем по жести
		time_svc_rtc_hardcore_init();
	}
}


int struct_tm_wday_to_rtc(int tm_wday, uint8_t * rtc_wday)
{
	// struct tm считает дни от 0 до 6 с воскресенья
	// RTC считает дни с 1 до 7 с понедельника

	int rc = 0;

	switch (tm_wday)
	{
	case 0: *rtc_wday = RTC_WEEKDAY_SUNDAY;		break;
	case 1: *rtc_wday = RTC_WEEKDAY_MONDAY; 		break;
	case 2: *rtc_wday = RTC_WEEKDAY_TUESDAY; 	break;
	case 3: *rtc_wday = RTC_WEEKDAY_WEDNESDAY; 	break;
	case 4: *rtc_wday = RTC_WEEKDAY_FRIDAY;		break;
	case 5: *rtc_wday = RTC_WEEKDAY_THURSDAY;	break;
	case 6: *rtc_wday = RTC_WEEKDAY_SATURDAY;	break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}


int rtc_wday_to_struct_tm(uint8_t rtc_wday, int * tm_wday)
{
	int rc = 0;

	switch (rtc_wday)
	{
	case RTC_WEEKDAY_SUNDAY:		*tm_wday = 0; break;
	case RTC_WEEKDAY_MONDAY:		*tm_wday = 1; break;
	case RTC_WEEKDAY_TUESDAY:	*tm_wday = 2; break;
	case RTC_WEEKDAY_WEDNESDAY:	*tm_wday = 3; break;
	case RTC_WEEKDAY_FRIDAY:		*tm_wday = 4; break;
	case RTC_WEEKDAY_THURSDAY:	*tm_wday = 5; break;
	case RTC_WEEKDAY_SATURDAY:	*tm_wday = 6; break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}


int struct_tm_year_to_rtc(int tm_year, uint8_t * rtc_year)
{
	// tm_year показывает количество лет с 1900-ого года
	// RTC работает в BCD кодировке с годами от 00 до 99
	int last_digits = (tm_year + 1900 - RTC_BASE_YEAR); // 1900 базовый год tm_year
	if (last_digits >= 100)
		return -EINVAL;

	*rtc_year = RTC_ByteToBcd2((uint8_t)last_digits);
	return 0;
}


int rtc_year_to_struct_tm(uint8_t rtc_year, int * tm_year)
{
	int last_digits = RTC_Bcd2ToByte(rtc_year);
	*tm_year = RTC_BASE_YEAR + last_digits - 1900; // 1900 базовый год tm_year
	return 0;
}


int struct_tm_month_to_rtc(int tm_month, uint8_t * rtc_month)
{
	// struct tm работает в номером месяца с 0 по 11ый
	// rtc работает в своей стремной BCD кодировке

	int rc = 0;
	switch (tm_month)
	{
	case 0:  *rtc_month = RTC_MONTH_JANUARY;	break;
	case 1:  *rtc_month = RTC_MONTH_FEBRUARY;	break;
	case 2:  *rtc_month = RTC_MONTH_MARCH;		break;
	case 3:  *rtc_month = RTC_MONTH_APRIL;		break;
	case 4:  *rtc_month = RTC_MONTH_MAY;		break;
	case 5:  *rtc_month = RTC_MONTH_JUNE;		break;
	case 6:  *rtc_month = RTC_MONTH_JULY;		break;
	case 7:  *rtc_month = RTC_MONTH_AUGUST;		break;
	case 8:  *rtc_month = RTC_MONTH_SEPTEMBER;	break;
	case 9:  *rtc_month = RTC_MONTH_OCTOBER;	break;
	case 10: *rtc_month = RTC_MONTH_NOVEMBER;	break;
	case 11: *rtc_month = RTC_MONTH_DECEMBER;	break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}


int rtc_month_to_struct_tm(uint8_t rtc_month, int * tm_month)
{
	int rc = 0;
	switch (rtc_month)
	{
	case RTC_MONTH_JANUARY:		*tm_month = 0;		break;
	case RTC_MONTH_FEBRUARY:	*tm_month = 1;		break;
	case RTC_MONTH_MARCH:		*tm_month = 2;		break;
	case RTC_MONTH_APRIL:		*tm_month = 3;		break;
	case RTC_MONTH_MAY:			*tm_month = 4;		break;
	case RTC_MONTH_JUNE:		*tm_month = 5;		break;
	case RTC_MONTH_JULY:		*tm_month = 6;		break;
	case RTC_MONTH_AUGUST:		*tm_month = 7;		break;
	case RTC_MONTH_SEPTEMBER:	*tm_month = 8;		break;
	case RTC_MONTH_OCTOBER:		*tm_month = 9;		break;
	case RTC_MONTH_NOVEMBER:	*tm_month = 10;		break;
	case RTC_MONTH_DECEMBER:	*tm_month = 11;		break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}


// https://stackoverflow.com/a/19377562/1869025
static int yisleap(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}


// https://stackoverflow.com/a/19377562/1869025
static int get_yday(int mon, int day, int year)
{
	static const int days[2][13] = {
		{0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		{0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
	};
	int leap = yisleap(year);

	return days[leap][mon] + day;
}


int rtc_to_struct_tm(const RTC_TimeTypeDef * rtc_time, const RTC_DateTypeDef * rtc_date, struct tm * tm)
{
	int rc = 0;

	rc = rtc_year_to_struct_tm(rtc_date->Year, &tm->tm_year);
	if (0 != rc)
		return rc;

	rc = rtc_month_to_struct_tm(rtc_date->Month, &tm->tm_mon);
	if (0 != rc)
		return rc;

	rc = rtc_wday_to_struct_tm(rtc_date->WeekDay, &tm->tm_wday);
	if (0 != rc)
		return rc;

	tm->tm_mday = RTC_Bcd2ToByte(rtc_date->Date);

	tm->tm_hour = RTC_Bcd2ToByte(rtc_time->Hours);
	tm->tm_min  = RTC_Bcd2ToByte(rtc_time->Minutes);
	tm->tm_sec =  RTC_Bcd2ToByte(rtc_time->Seconds);

	tm->tm_isdst = 0;
	tm->tm_yday = get_yday(tm->tm_mon, tm->tm_mday, tm->tm_year + 1900);

	return 0;
}


int struct_tm_to_rtc(const struct tm * tm, RTC_TimeTypeDef * rtc_time, RTC_DateTypeDef * rtc_date)
{
	int rc = 0;

	rc = struct_tm_year_to_rtc(tm->tm_year, &rtc_date->Year);
	if (0 != rc)
		return rc;

	rc = struct_tm_month_to_rtc(tm->tm_mon, &rtc_date->Month);
	if (0 != rc)
		return rc;

	rc = struct_tm_wday_to_rtc(tm->tm_wday, &rtc_date->WeekDay);
	if (0 != rc)
		return rc;

	rtc_date->Date = RTC_ByteToBcd2(tm->tm_mday);

	rtc_time->Hours = RTC_ByteToBcd2(tm->tm_hour);
	rtc_time->Minutes = RTC_ByteToBcd2(tm->tm_min);
	rtc_time->Seconds = RTC_ByteToBcd2(tm->tm_sec);

	rtc_time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	rtc_time->StoreOperation = RTC_STOREOPERATION_RESET;
	rtc_time->SecondFraction = 0;
	rtc_time->SubSeconds = 0;

	return 0;
}

