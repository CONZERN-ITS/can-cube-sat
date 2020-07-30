#ifndef SRC_INTEGERATED_TEMPERATURE_H_
#define SRC_INTEGERATED_TEMPERATURE_H_


#include <mavlink_main.h>


//! Получение пакета данных с интегрированных в чип сенсоров
/*! АЦП должно быть уже включено */
int integrated_read(mavlink_own_temp_t * msg);


#endif /* SRC_INTEGERATED_TEMPERATURE_H_ */
