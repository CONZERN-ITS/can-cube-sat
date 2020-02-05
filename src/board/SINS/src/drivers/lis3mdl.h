/*
* lis3mdl.h
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#ifndef DRIVERS_LIS3MDL_H_
#define DRIVERS_LIS3MDL_H_
#include <stdint.h>

#include "state.h"
#include "lis3mdl_reg.h"

int lis3mdl_init(void);

uint32_t lis3mdl_get_m_data_mG(float* magn);

#endif /* DRIVERS_LIS3MDL_H_ */
