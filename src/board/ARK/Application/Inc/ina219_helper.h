/*
 * pcu_inahelpers.h
 *
 *  Created on: Jun 10, 2019
 *      Author: kirs
 */


#include "ina219.h"

#include <stdio.h>

#include "main.h"

int ina219_init_default(ina219_t * self, I2C_HandleTypeDef *hi2c, ina219_i2c_addr_t addr, uint32_t timeout);

// Current is given in amperes, power is in watts
int ina219_read(ina219_t * self, float * current, float * power);

float ina219_power_convert(ina219_t * hina, uint16_t power);

float ina219_current_convert(ina219_t * hina, int16_t current);

float ina219_bus_voltage_convert(ina219_t * hina, uint16_t voltage);

float ina219_shunt_voltage_convert(ina219_t * hina, int16_t voltage);

