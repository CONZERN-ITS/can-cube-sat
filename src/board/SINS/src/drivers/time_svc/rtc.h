/*
 * hardcore_rtc_init.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_RTC_H_
#define DRIVERS_TIME_SVC_RTC_H_

#include <time.h>
#include <stm32f4xx_hal.h>

extern RTC_HandleTypeDef hrtc;


//! Запуск RTC по хардкору.
/*! Настриваем вообще все, что нужно настраивать в бекап домене.
 *  Включаем и настраиваем LSE. Включаем RTC. Настраиваем его на эпоху GPS и запускаем */
int time_svc_rtc_hardcore_init(void);

//! Настройка RTC по-нормальному
/*! Предполагается что RTC уже настроено кем-то другим и настроено так как мы ожидаем
 *  Мы просто подключаемся к нему и работаем вообще его не трогая */
int time_svc_rtc_simple_init(void);

//! Настройка RTC в общем
/*! Настраивать понрмальному или по-хардкору решает само. Просто глядя работает ли уже RTC или еще нет */
int time_svc_rtc_init();


//! Загрузка времени из RTC в struct_tm
int time_svc_rtc_load(struct tm * tm);

//! Сохранение времени из struct tm в RTC
int time_svc_rtc_store(const struct tm * tm);


//! Установка времени в будильник RTC
/*! Будильник RTC довольно наивный и по абсолютному времени не работает.
 *  Самое близкое это точное время на определенный день какого-то месяца какого-то года.
 *  Аргумент alarm должен быть из множества RTC_Alarms_Definitions. RTC_ALARM_A или RTC_ALARM_B */
int time_svc_rtc_alarm_setup(const struct tm * tm, uint32_t alarm);


#endif /* DRIVERS_TIME_SVC_RTC_H_ */
