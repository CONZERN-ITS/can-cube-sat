/*
 * bme280.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */

#include <stm32f1xx_hal.h>

#include <bme280.h>

#include "util.h"


extern I2C_HandleTypeDef hi2c2;

#define ITS_PLD_BME280_BUS_HANDLE hi2c2
#define ITS_PLD_BME280_I2C_ADDR BME280_I2C_ADDR_PRIM
#define ITS_PLD_BME280_HAL_TIMEOUT (1000)

static struct bme280_dev _device;


static void _delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}



static int8_t _i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	HAL_StatusTypeDef hrc = HAL_I2C_Mem_Read(
			&ITS_PLD_BME280_BUS_HANDLE,
			ITS_PLD_BME280_I2C_ADDR,
			reg_addr,
			1,
			data,
			len,
			ITS_PLD_BME280_HAL_TIMEOUT
	);

	int rc = its_pld_hal_status_to_errno(hrc);
	return rc;
}


static int8_t _i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	HAL_StatusTypeDef hrc = HAL_I2C_Mem_Write(
			&ITS_PLD_BME280_BUS_HANDLE,
			ITS_PLD_BME280_I2C_ADDR,
			reg_addr,
			1,
			data,
			len,
			ITS_PLD_BME280_HAL_TIMEOUT
	);

	int rc = its_pld_hal_status_to_errno(hrc);
	return rc;
}



int its_pld_bme280_init()
{
	_device.intf = BME280_I2C_INTF;
	_device.read = _i2c_read;
	_device.write = _i2c_write;
	_device.delay_ms = _delay_ms;

	int rc = bme280_init(&_device);
}


