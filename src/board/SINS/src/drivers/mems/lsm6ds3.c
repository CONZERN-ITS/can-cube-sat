/*
 * lsm6ds3.c
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#include "lsm6ds3.h"

#include <stm32f4xx_hal.h>
#include <diag/Trace.h>

#include <assert.h>
#include <errno.h>

#include "../common.h"
#include "lsm6ds3_reg.h"
#include "mems.h"
#include "vector.h"


#define LSM_TIMEOUT	1000
#define MG_TO_MPS2	9.80665 / 1000
#define MDPS_TO_RAD	M_PI / 180 / 1000


//	Accelerometer bias & transform matrix
#define X_ACCEL_OFFSET		0.000000
#define Y_ACCEL_OFFSET		0.000000
#define Z_ACCEL_OFFSET		0.000000
#define XX_ACCEL_TRANSFORM_MATIX	 1.000000
#define YY_ACCEL_TRANSFORM_MATIX	 1.000000
#define ZZ_ACCEL_TRANSFORM_MATIX	 1.000000
#define XY_ACCEL_TRANSFORM_MATIX	 0.000000
#define XZ_ACCEL_TRANSFORM_MATIX	 0.000000
#define YZ_ACCEL_TRANSFORM_MATIX	 0.000000

#define LSM6DS3_I2C_READ_ADD	0b11010111
#define LSM6DS3_I2C_WRITE_ADD	0b11010110

static const float offset_vector[3] = {X_ACCEL_OFFSET, Y_ACCEL_OFFSET, Z_ACCEL_OFFSET};
static const float transform_matrix[3][3] =	{
		{ XX_ACCEL_TRANSFORM_MATIX, XY_ACCEL_TRANSFORM_MATIX, XZ_ACCEL_TRANSFORM_MATIX },
		{ XY_ACCEL_TRANSFORM_MATIX, YY_ACCEL_TRANSFORM_MATIX, YZ_ACCEL_TRANSFORM_MATIX },
		{ XZ_ACCEL_TRANSFORM_MATIX, YZ_ACCEL_TRANSFORM_MATIX, ZZ_ACCEL_TRANSFORM_MATIX }
};



typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;


static int32_t lsm6ds3_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lsm6ds3_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

static stmdev_ctx_t lsm6ds3_dev_ctx = {
		.write_reg = lsm6ds3_write,
		.read_reg = lsm6ds3_read,
		.handle = &hmems_i2c
};



int mems_lsm6ds3_init(void)
{
	int error = 0;
	uint8_t whoamI = 0;

	lsm6ds3_ctrl3_c_t reg = {0};
	reg.sw_reset = 1;
	error = lsm6ds3_write_reg(&lsm6ds3_dev_ctx, LSM6DS3_CTRL3_C, (uint8_t*)&reg, 1);
	if (error != 0)
		return error;
	error = lsm6ds3_i2c_interface_set(&lsm6ds3_dev_ctx, LSM6DS3_I2C_ENABLE);
	if (error != 0)
		return error;

	// Check who_am_i
	error = lsm6ds3_device_id_get(&lsm6ds3_dev_ctx, &whoamI);
	if (whoamI != LSM6DS3_ID)
	{
//		trace_printf("lsm6ds3 not found, %d\terror: %d\n", whoamI, error);
		return -ENODEV;
	}

//	trace_printf("lsm6ds3 OK\n");

	error = lsm6ds3_block_data_update_set(&lsm6ds3_dev_ctx, PROPERTY_ENABLE);
	if (0 != error) return error;

	error = lsm6ds3_xl_full_scale_set(&lsm6ds3_dev_ctx, LSM6DS3_4g);
	if (0 != error) return error;

	error = lsm6ds3_gy_full_scale_set(&lsm6ds3_dev_ctx, LSM6DS3_1000dps);
	if (0 != error) return error;

	error = lsm6ds3_xl_data_rate_set(&lsm6ds3_dev_ctx, LSM6DS3_XL_ODR_104Hz);
	if (0 != error) return error;

	error = lsm6ds3_gy_data_rate_set(&lsm6ds3_dev_ctx, LSM6DS3_GY_ODR_104Hz);
	if (0 != error) return error;

	error = lsm6ds3_xl_filter_analog_set(&lsm6ds3_dev_ctx, LSM6DS3_ANTI_ALIASING_200Hz);
	if (0 != error) return error;

//	error = lsm6ds3_xl_power_mode_set(&lsm6ds3_dev_ctx, LSM6DS3_XL_HIGH_PERFORMANCE);
//	if (0 != error) return error;

//	error = lsm6ds3_gy_power_mode_set(&lsm6ds3_dev_ctx, LSM6DS3_GY_HIGH_PERFORMANCE);
//	if (0 != error) return error;

	return 0;
}


int mems_lsm6ds3_get_xl_data_g(int16_t * data, float * accel)
{

	accel[0] = lsm6ds3_from_fs4g_to_mg(data[0]) * MG_TO_MPS2;
	accel[1] = lsm6ds3_from_fs4g_to_mg(data[1]) * MG_TO_MPS2;
	accel[2] = lsm6ds3_from_fs4g_to_mg(data[2]) * MG_TO_MPS2;

#if !CALIBRATION
		//	Accelerometer bias and transform matrix (to provide real values)

		vmv(accel, (float*)offset_vector, accel);
		mxv((float(*)[3])transform_matrix, accel, accel);

//		float tmp = accel[0];
//		accel[0] = accel[1];
//		accel[1] = tmp;
//		accel[2] = -accel[2];
#endif

	return 0;
}


int mems_lsm6ds3_get_xl_data_raw(int16_t * data)
{
	axis3bit16_t data_raw_acceleration;
	int error;
	//	Read acceleration field data
	error = lsm6ds3_acceleration_raw_get(&lsm6ds3_dev_ctx, data_raw_acceleration.u8bit);
	if (0 != error) return error;

	for (int i = 0; i < 3; i++)
		data[i] = data_raw_acceleration.i16bit[i];

	return 0;
}


int mems_lsm6ds3_get_g_data_rps(int16_t * data, float* gyro)
{
	gyro[0] = lsm6ds3_from_fs1000dps_to_mdps(data[0]) * MDPS_TO_RAD;
	gyro[1] = lsm6ds3_from_fs1000dps_to_mdps(data[1]) * MDPS_TO_RAD;
	gyro[2] = lsm6ds3_from_fs1000dps_to_mdps(data[2]) * MDPS_TO_RAD;

	float tmp = gyro[0];
	gyro[0] = gyro[1];
	gyro[1] = tmp;
	gyro[2] = -gyro[2];
	return 0;
}


int mems_lsm6ds3_get_g_data_raw(int16_t * data)
{
	int error;
	axis3bit16_t data_raw_angular_rate;
	//	Read acceleration field data
	error = lsm6ds3_angular_rate_raw_get(&lsm6ds3_dev_ctx, data_raw_angular_rate.u8bit);
	if (0 != error) return error;

	for (int i = 0; i < 3; i++)
		data[i] = data_raw_angular_rate.i16bit[i];

	return 0;
}


/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t lsm6ds3_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
	HAL_StatusTypeDef hal_error;

	hal_error = HAL_I2C_Mem_Write(handle, LSM6DS3_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, LSM_TIMEOUT);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t lsm6ds3_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
	HAL_StatusTypeDef hal_error;

	hal_error = HAL_I2C_Mem_Read(handle, LSM6DS3_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, LSM_TIMEOUT);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	return 0;
}
