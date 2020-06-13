/*
 * bme280.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */

#include "sensors/bme.h"

#include <stm32f1xx_hal.h>

#include <bme280.h>

#include "time_svc.h"
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
			ITS_PLD_BME280_I2C_ADDR << 1,
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
			ITS_PLD_BME280_I2C_ADDR << 1,
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
	if (0 != rc)
		return rc;

	_device.settings.filter = BME280_FILTER_COEFF_OFF;
	_device.settings.osr_h = BME280_OVERSAMPLING_16X;
	_device.settings.osr_p = BME280_OVERSAMPLING_16X;
	_device.settings.osr_t = BME280_OVERSAMPLING_16X;
	_device.settings.standby_time = BME280_STANDBY_TIME_500_MS;
	rc = bme280_set_sensor_settings(BME280_ALL_SETTINGS_SEL, &_device);
	if (0 != rc)
		return rc;

	rc = bme280_set_sensor_mode(BME280_NORMAL_MODE, &_device);
	if (0 != rc)
		return rc;

	return 0;
}


int its_pld_bme280_read(mavlink_pld_bme280_data_t * data)
{
	struct bme280_data bme280_data;

	struct timeval tv;
	time_svc_gettimeofday(&tv);

	int rc = bme280_get_sensor_data(BME280_ALL, &bme280_data, &_device);
	if (0 != rc)
		return rc;

	data->time_s = tv.tv_sec;
	data->time_us = tv.tv_usec;
	data->pressure = bme280_data.pressure;
	data->temperature = bme280_data.temperature;
	data->humidity = bme280_data.humidity;
	data->altitude = 0;
	return 0;
}
