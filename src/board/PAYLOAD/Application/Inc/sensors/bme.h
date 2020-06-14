/*
 * bme280.h
 *
 *  Created on: 18 мая 2020 г.
 *      Author: snork
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_

#include <mavlink_main.h>


//! Инициализация сенсора
int bme_init(void);

//! Реинициализация сенсора.
//! Сброс всего до чего можно дотянуться и повторная попытка инициализации
int bme_restart(void);

//! Получение mavlink пакета с данными BME
int bme_read(mavlink_pld_bme280_data_t * data);


#endif /* INC_BME280_H_ */
