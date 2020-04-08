/*
 * time_util.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_TIME_UTIL_H_
#define DRIVERS_TIME_SVC_TIME_UTIL_H_


#include <stdint.h>
#include <time.h>

#include <stm32f4xx_hal.h>

//! Перевод времени GPS в UNIX время
struct timeval gps_time_to_unix_time(uint16_t week, uint32_t tow_ms);


//! Пересчет дня недели из терминов struct tm в термины RTC
int struct_tm_wday_to_rtc(int tm_wday, uint8_t * rtc_wday);
//! Пересчет дня недели из терминов RTC в термины struct tm
int rtc_wday_to_struct_tm(uint8_t rtc_wday, int * tm_wday);

//! Пересчет номера месяца из терминов struct_tm в термины RTC
int struct_tm_month_to_rtc(int tm_month, uint8_t * rtc_month);
//! Пересчет номера месяца из терминов RTC в термины struct_tm
int rtc_month_to_struct_tm(uint8_t rtc_month, int * tm_month);

//! Пересчет номера года из терминов struct_tm в термины RTC
int struct_tm_year_to_rtc(int tm_year, uint8_t * rtc_year);
//! Пересчет номера года из терминов RTC в термины struct_tm
int rtc_year_to_struct_tm(uint8_t rtc_year, int * tm_year);

//! Сборка struct tm из RTCшных структур хала
/*! Подразумевается BCD формат на 24 часа.
 *  Daylight Saving, SecondFraction и SubSeconds не учитываются */
int rtc_to_struct_tm(const RTC_TimeTypeDef * rtc_time, const RTC_DateTypeDef * rtc_date, struct tm * tm);

//! Сборка RTCшных структур хала из struct tm
/*! Подразумевается BCD формат на 24 часа.
 *  DaylightSaving = RTC_DAYLIGHTSAVING_NONE,
 *  StoreOperation = RTC_STOREOPERATION_RESET,
 *  SecondFraction = 0,
 *  SubSeconds = 0 */
int struct_tm_to_rtc(const struct tm * tm, RTC_TimeTypeDef * rtc_time, RTC_DateTypeDef * rtc_date);


#endif /* DRIVERS_TIME_SVC_TIME_UTIL_H_ */
