/*
 * time_util.c
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#include "time_util.h"

#include <assert.h>
#include <errno.h>

#include <stm32f4xx_ll_rtc.h>


//! Базовый год для RTC, так как оно работает в двух последних цифра
#define RTC_BASE_YEAR 2000
#define SECONDS_PER_WEEK (60 * 60 * 24 * 7)
#define GPS_EPOCH_IN_UNIX_TIME (315964800)


void gps_time_to_unix_time(uint16_t week, uint32_t tow_ms, struct timeval * tmv)
{
	tmv->tv_sec = week * SECONDS_PER_WEEK + tow_ms / 1000 + GPS_EPOCH_IN_UNIX_TIME;
	tmv->tv_usec = (tow_ms % 1000) * 1000;
}


void unix_time_to_gps_time(const struct timeval * tmv, uint16_t * week, uint32_t * tow_ms)
{
	const time_t gps_epoch_seconds = tmv->tv_sec - GPS_EPOCH_IN_UNIX_TIME;

	*week = gps_epoch_seconds / SECONDS_PER_WEEK;
	*tow_ms = (gps_epoch_seconds % SECONDS_PER_WEEK) * 1000 + tmv->tv_usec / 1000;
}



//! Пересчет дня недели из терминов struct tm в термины RTC
int struct_tm_wday_to_rtc(int tm_wday, uint8_t * rtc_wday)
{
	// struct tm считает дни от 0 до 6 с воскресенья
	// RTC считает дни с 1 до 7 с понедельника

	int rc = 0;

	switch (tm_wday)
	{
	case 0: *rtc_wday = LL_RTC_WEEKDAY_SUNDAY;		break;
	case 1: *rtc_wday = LL_RTC_WEEKDAY_MONDAY; 		break;
	case 2: *rtc_wday = LL_RTC_WEEKDAY_TUESDAY; 	break;
	case 3: *rtc_wday = LL_RTC_WEEKDAY_WEDNESDAY; 	break;
	case 4: *rtc_wday = LL_RTC_WEEKDAY_FRIDAY;		break;
	case 5: *rtc_wday = LL_RTC_WEEKDAY_THURSDAY;	break;
	case 6: *rtc_wday = LL_RTC_WEEKDAY_SATURDAY;	break;

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
	case LL_RTC_WEEKDAY_SUNDAY:		*tm_wday = 0; break;
	case LL_RTC_WEEKDAY_MONDAY:		*tm_wday = 1; break;
	case LL_RTC_WEEKDAY_TUESDAY:	*tm_wday = 2; break;
	case LL_RTC_WEEKDAY_WEDNESDAY:	*tm_wday = 3; break;
	case LL_RTC_WEEKDAY_FRIDAY:		*tm_wday = 4; break;
	case LL_RTC_WEEKDAY_THURSDAY:	*tm_wday = 5; break;
	case LL_RTC_WEEKDAY_SATURDAY:	*tm_wday = 6; break;

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

	*rtc_year = __LL_RTC_CONVERT_BIN2BCD((uint8_t)last_digits);
	return 0;
}


int rtc_year_to_struct_tm(uint8_t rtc_year, int * tm_year)
{
	int last_digits = __LL_RTC_CONVERT_BCD2BIN(rtc_year);
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
	case 0:  *rtc_month = LL_RTC_MONTH_JANUARY;		break;
	case 1:  *rtc_month = LL_RTC_MONTH_FEBRUARY;	break;
	case 2:  *rtc_month = LL_RTC_MONTH_MARCH;		break;
	case 3:  *rtc_month = LL_RTC_MONTH_APRIL;		break;
	case 4:  *rtc_month = LL_RTC_MONTH_MAY;			break;
	case 5:  *rtc_month = LL_RTC_MONTH_JUNE;		break;
	case 6:  *rtc_month = LL_RTC_MONTH_JULY;		break;
	case 7:  *rtc_month = LL_RTC_MONTH_AUGUST;		break;
	case 8:  *rtc_month = LL_RTC_MONTH_SEPTEMBER;	break;
	case 9:  *rtc_month = LL_RTC_MONTH_OCTOBER;		break;
	case 10: *rtc_month = LL_RTC_MONTH_NOVEMBER;	break;
	case 11: *rtc_month = LL_RTC_MONTH_DECEMBER;	break;

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
	case LL_RTC_MONTH_JANUARY:		*tm_month = 0;		break;
	case LL_RTC_MONTH_FEBRUARY:		*tm_month = 1;		break;
	case LL_RTC_MONTH_MARCH:		*tm_month = 2;		break;
	case LL_RTC_MONTH_APRIL:		*tm_month = 3;		break;
	case LL_RTC_MONTH_MAY:			*tm_month = 4;		break;
	case LL_RTC_MONTH_JUNE:			*tm_month = 5;		break;
	case LL_RTC_MONTH_JULY:			*tm_month = 6;		break;
	case LL_RTC_MONTH_AUGUST:		*tm_month = 7;		break;
	case LL_RTC_MONTH_SEPTEMBER:	*tm_month = 8;		break;
	case LL_RTC_MONTH_OCTOBER:		*tm_month = 9;		break;
	case LL_RTC_MONTH_NOVEMBER:		*tm_month = 10;		break;
	case LL_RTC_MONTH_DECEMBER:		*tm_month = 11;		break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}


int rtc_store(RTC_TypeDef * hrtc, const struct tm * tm)
{
	int rc = 0;

	uint8_t rtc_wday;
	rc = struct_tm_wday_to_rtc(tm->tm_wday, &rtc_wday);
	if (0 != rc)
		return rc;

	uint8_t rtc_mon;
	rc = struct_tm_month_to_rtc(tm->tm_mon, &rtc_mon);
	if (0 != rc)
		return rc;

	uint8_t rtc_year;
	rc = struct_tm_year_to_rtc(tm->tm_year, &rtc_year);
	if (0 != rc)
		return rc;

	LL_RTC_DATE_Config(hrtc,
			rtc_wday,
			// удивительно, но тут мы совпали. Цифры одинаковые
			__LL_RTC_CONVERT_BIN2BCD(tm->tm_mday),
			rtc_mon,
			rtc_year
	);

	LL_RTC_TIME_Config(hrtc,
			LL_RTC_TIME_FORMAT_AM_OR_24,
			__LL_RTC_CONVERT_BIN2BCD(tm->tm_hour),
			__LL_RTC_CONVERT_BIN2BCD(tm->tm_min),
			__LL_RTC_CONVERT_BIN2BCD(tm->tm_sec)
	);

	return 0;
}


int rtc_load(RTC_TypeDef * hrtc, struct tm * tm)
{
	int rc = 0;

	uint32_t combined_date = LL_RTC_DATE_Get(hrtc);
	uint32_t combined_time = LL_RTC_TIME_Get(hrtc);

	uint8_t rtc_year = __LL_RTC_GET_YEAR(combined_date);
	uint8_t rtc_mon  = __LL_RTC_GET_MONTH(combined_date);
	uint8_t rtc_wday = __LL_RTC_GET_WEEKDAY(combined_date);

	rc = rtc_year_to_struct_tm(rtc_year, &tm->tm_year);
	if (0 != rc)
		return rc;

	rc = rtc_month_to_struct_tm(rtc_mon, &tm->tm_mon);
	if (0 != rc)
		return rc;

	rc = rtc_wday_to_struct_tm(rtc_wday, &tm->tm_wday);
	if (0 != rc)
		return rc;

	tm->tm_mday = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_DAY(combined_date));

	tm->tm_hour = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_HOUR(combined_time));
	tm->tm_min  = __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_MINUTE(combined_time));
	tm->tm_sec =  __LL_RTC_CONVERT_BCD2BIN(__LL_RTC_GET_SECOND(combined_time));

	tm->tm_isdst = 0;

	return 0;
}
