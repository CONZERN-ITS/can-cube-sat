/*
 * time_util.h
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#ifndef INC_TIME_UTIL_H_
#define INC_TIME_UTIL_H_

#include <stdint.h>
#include <time.h>

#include <stm32f4xx_ll_rtc.h>

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

//! Установка времени в RTC
/*! Подразумевается формат 24 часа.
 *  Перед вызовом RTC должен быть разблокирован и переведен в режим настройки */
int rtc_store(RTC_TypeDef * hrtc, const struct tm * tm);

//! Чтение времени из RTC
int rtc_load(RTC_TypeDef * hrtc, struct tm * tm);

#endif /* INC_TIME_UTIL_H_ */
