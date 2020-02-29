/*
 * pcu_inahelpers.h
 *
 *  Created on: Jun 10, 2019
 *      Author: kirs
 */


#include "ina219.h"

#include <stdio.h>

#include "main.h"

//#include "delay.h"

#define INA_CURRENT_LSB	        ((ina219_float_t)(1.f/0x8000)) // считаем из расчета на 3 ампер максимум
#define INA_POWER_LSB	        ((ina219_float_t)(20*INA_CURRENT_LSB))
#define INA_VOLTAGE_BUS_LSB     0.004
#define INA_VOLTAGE_SHUNT_LSB   0.01

static int _ina_init(ina219_t * self, I2C_HandleTypeDef *hi2c, ina219_i2c_addr_t addr) {
	int error = 0;

	ina219_init(self, hi2c, addr);

	error = ina219_sw_reset(self);
	// подождем после резета
	HAL_Delay(10);


	uint16_t cfgreg;
	error = ina219_read_reg(self, 0x00, &cfgreg);
	//trace_printf("initial cfgreg: 0x%04"PRIX16", error: %d\n", cfgreg, error);

	ina219_cfg_t ina_cfg;
	ina_cfg.bus_range = INA219_BUS_RANGE_16V;
	ina_cfg.bus_res = INA219_ADC_RES_12_BIT_OVS_128;
	ina_cfg.shunt_range = INA219_SHUNT_RANGE_40MV;
	ina_cfg.shunt_res = INA219_ADC_RES_12_BIT_OVS_1;
	ina_cfg.mode = INA219_MODE_SHUNT_AND_BUS_CONT;

	ina_cfg.shunt_r = 0.1f; // 100 миллиом
	ina_cfg.current_lsb = INA_CURRENT_LSB;
	error = ina219_set_cfg(self, &ina_cfg);
	printf("set_cfg error: %d\n", error);

	error = ina219_read_reg(self, 0x00, &cfgreg);
	//trace_printf("setted cfgreg: 0x%04"PRIX16" , error: %d\n", cfgreg, error);

	return error;
}

// Current is given in amperes, power is in watts
static int _ina_read(ina219_t * self, float * current, float * power)
{
	ina219_secondary_data_t data;
	int error;

	error = ina219_read_secondary(self, &data);
	if (error) {
	    printf("INA 219 read error %d\n", error);
	}

	*current = data.current * INA_CURRENT_LSB;
	*power = data.power * INA_POWER_LSB;

	return error;
}

static float _ina_power_convert(ina219_t * hina, uint16_t power) {
    return power * INA_POWER_LSB;
}
static float _ina_current_convert(ina219_t * hina, int16_t current) {
    return current * INA_CURRENT_LSB;
}
static float _ina_bus_voltage_convert(ina219_t * hina, uint16_t voltage) {
    return voltage * INA_VOLTAGE_BUS_LSB;
}
static float _ina_shunt_voltage_convert(ina219_t * hina, int16_t voltage) {
    return voltage * INA_VOLTAGE_SHUNT_LSB;
}
