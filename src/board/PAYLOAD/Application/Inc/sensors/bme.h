/*
 * bme280.h
 *
 *  Created on: 18 мая 2020 г.
 *      Author: snork
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_

#include <mavlink_main.h>

int its_pld_bme280_init(void);

int its_pld_bme280_read(mavlink_pld_bme280_data_t * data);


#endif /* INC_BME280_H_ */
