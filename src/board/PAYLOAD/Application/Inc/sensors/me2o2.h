/*
 * me2o2.h
 *
 *  Created on: 31 мая 2020 г.
 *      Author: snork
 */

#ifndef INC_ME2O2_H_
#define INC_ME2O2_H_

#include <mavlink_main.h>

//! Получение пакета данных от me2o2
/*! АЦП должно быть уже включено */
int me2o2_read(mavlink_pld_me2o2_data_t * msg);


#endif /* INC_ME2O2_H_ */
