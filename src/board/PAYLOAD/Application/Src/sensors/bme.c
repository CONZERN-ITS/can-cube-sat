/*
 * bme280.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */

#include "sensors/bme.h"

#include <assert.h>

#include <stm32f1xx_hal.h>

#include <bme280.h>

#include "time_svc.h"
#include "util.h"



extern I2C_HandleTypeDef hi2c2;

#define BME_BUS_HANDLE &hi2c2
#define BME_BUS_RCC_FORCE_RESET __HAL_RCC_I2C2_FORCE_RESET
#define BME_BUS_RCC_RELEASE_RESET __HAL_RCC_I2C2_RELEASE_RESET

#define BME_I2C_ADDR BME280_I2C_ADDR_PRIM
#define BME_HAL_TIMEOUT (300)


#ifndef ITS_IMITATOR
static struct bme280_dev _device;


static void _delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}


static int8_t _i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	HAL_StatusTypeDef hrc = HAL_I2C_Mem_Read(
			BME_BUS_HANDLE,
			BME_I2C_ADDR << 1,
			reg_addr,
			1,
			data,
			len,
			BME_HAL_TIMEOUT
	);

	int rc = hal_status_to_errno(hrc);
	return rc;
}


static int8_t _i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
	HAL_StatusTypeDef hrc = HAL_I2C_Mem_Write(
			BME_BUS_HANDLE,
			BME_I2C_ADDR << 1,
			reg_addr,
			1,
			data,
			len,
			BME_HAL_TIMEOUT
	);

	int rc = hal_status_to_errno(hrc);
	return rc;
}
#endif

#ifdef ITS_IMITATOR
int bme_init()
{
	return 0;
}
#else
int bme_init()
{
	_device.intf = BME280_I2C_INTF;
	_device.read = _i2c_read;
	_device.write = _i2c_write;
	_device.delay_ms = _delay_ms;

	int rc = bme280_soft_reset(&_device);
	if (0 != rc)
		return rc;

	rc = bme280_init(&_device);
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

	HAL_Delay(10); // Иначе первые данные получаются плохие
	return 0;
}
#endif


#ifdef ITS_IMITATOR
int bme_restart()
{
	return 0;
}
#else
int bme_restart()
{
	I2C_HandleTypeDef * bus_handle = BME_BUS_HANDLE;

	// Глушим I2C
	// Вот это - очень важно. Без него оно встает наглухо
	// даже через RCC_RELEASE_RESET
	if (READ_BIT(bus_handle->Instance->CR1, I2C_CR1_SWRST))
		CLEAR_BIT(bus_handle->Instance->CR1, I2C_CR1_SWRST);

	HAL_I2C_DeInit(bus_handle);
	BME_BUS_RCC_FORCE_RESET();
	HAL_Delay(1);
	BME_BUS_RCC_RELEASE_RESET();
	__HAL_I2C_RESET_HANDLE_STATE(bus_handle);

	HAL_StatusTypeDef hal_rc;
	hal_rc = HAL_I2C_Init(bus_handle);
	int rc = hal_status_to_errno(hal_rc);;
	if (0 != rc)
		return rc;

	return bme_init();
}
#endif


#ifdef ITS_IMITATOR
int bme_read(mavlink_pld_bme280_data_t * data)
{
	struct timeval tv;
	time_svc_gettimeofday(&tv);

	data->time_s = tv.tv_sec;
	data->time_us = tv.tv_usec;
	data->pressure = 100*1000;
	data->temperature = 36.6;
	data->humidity = 42;
	data->altitude = 0;

	return 0;
}
#else
int bme_read(mavlink_pld_bme280_data_t * data)
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
#endif
