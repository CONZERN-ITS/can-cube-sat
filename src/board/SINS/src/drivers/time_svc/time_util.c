/*
 * time_util.c
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#include "time_util.h"

#include <assert.h>
#include <errno.h>

//! Базовый год для RTC, так как оно работает в двух последних цифрах года
#define RTC_BASE_YEAR 2000


//! Перевод времени GPS в UNIX время
inline struct timeval gps_time_to_unix_time(uint16_t week, uint32_t tow_ms)
{
	struct timeval rv;
	rv.tv_sec = week * 60 * 60 * 24 * 7 + tow_ms / 1000;
	rv.tv_usec = (tow_ms % 1000) * 1000;

	rv.tv_sec += 315964800; // эпоха GPS в UNIX времени
	return rv;
}


//! Пересчет дня недели из терминов struct tm в термины RTC
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
