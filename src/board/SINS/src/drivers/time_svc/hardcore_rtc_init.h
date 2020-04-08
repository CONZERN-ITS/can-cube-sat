/*
 * hardcore_rtc_init.h
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#ifndef DRIVERS_TIME_SVC_HARDCORE_RTC_INIT_H_
#define DRIVERS_TIME_SVC_HARDCORE_RTC_INIT_H_


//! Запуск RTC по хардкору.
/*! Настриваем вообще все, что нужно настраивать в бекап домене.
 *  Включаем и настраиваем LSE. Включаем и настраиваем RTC */
int time_svc_hardcore_its_init(void);


#endif /* DRIVERS_TIME_SVC_HARDCORE_RTC_INIT_H_ */
