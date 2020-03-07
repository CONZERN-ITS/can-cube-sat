/*
 * strela-ms-rpi.c
 *
 *  Created on: Feb 21, 2020
 *      Author: developer
 */
#include "unistd.h"
#include "lis3mdl_STdC/driver/lis3mdl_reg.h"
#include "lsm6ds3_STdC/driver/lsm6ds3_reg.h"
typedef enum {
	ERROR_NO_ERROR = 0,
	ERROR_UNKNOWN_ID = -1
} error_t;

int32_t lis3mdl_init (stmdev_ctx_t *lis3mdl_context){
	int32_t error = 0;
	uint8_t reset = 1;
	uint8_t id = 0;

	// Reset
    error = lis3mdl_reset_set(lis3mdl_context, PROPERTY_ENABLE);
	if (error) return error;
	usleep(50*1000);

	// Check id
	error = lis3mdl_device_id_get(lis3mdl_context, &id);
	if (error) return error;
	if (id != LIS3MDL_ID) return ERROR_UNKNOWN_ID;

	// Set configuration
	error = lis3mdl_block_data_update_set(lis3mdl_context, PROPERTY_ENABLE);
	if (error) return error;
	error = lis3mdl_data_rate_set(lis3mdl_context, LIS3MDL_HP_40Hz);
	if (error) return error;
	error = lis3mdl_full_scale_set(lis3mdl_context, LIS3MDL_16_GAUSS);
	if (error) return error;
	error = lis3mdl_operating_mode_set(lis3mdl_context, LIS3MDL_CONTINUOUS_MODE);
	return error;
}

int32_t lis3mdl_get_data_G (stmdev_ctx_t *lis3mdl_context, float* mag){
	uint8_t data_raw [6];
	uint8_t error;

	error = lis3mdl_magnetic_raw_get(lis3mdl_context, data_raw);
	if (error) return error;
	mag[0] = LIS3MDL_FROM_FS_16G_TO_G(((data_raw[1] << 8) | data_raw[0]));
	mag[1] = LIS3MDL_FROM_FS_16G_TO_G(((data_raw[3] << 8) | data_raw[2]));
	mag[2] = LIS3MDL_FROM_FS_16G_TO_G(((data_raw[5] << 8) | data_raw[4]));
	return 0;
}

int32_t lsm6ds3_init(stmdev_ctx_t *lsm6ds3_context){
	int32_t error = 0;
	uint8_t reset = 1;
	uint8_t id = 0;

	// Reset
	lsm6ds3_reg_t reg = {0};
	reg.ctrl3_c.sw_reset = 1;
	error = lsm6ds3_write_reg(lsm6ds3_context, LSM6DS3_CTRL3_C, &reg.byte, 1);
	if (error) return error;
	usleep(50*1000);

	// Check id
	error = lsm6ds3_device_id_get(lsm6ds3_context, &id);
	if (error) return error;
	if (id != LSM6DS3_ID) return ERROR_UNKNOWN_ID;

	// Set configuration
	error = lsm6ds3_block_data_update_set(lsm6ds3_context, PROPERTY_ENABLE);
	if (error) return error;
	error = lsm6ds3_xl_full_scale_set(lsm6ds3_context, LSM6DS3_4g);
	if (error) return error;
	error = lsm6ds3_gy_full_scale_set(lsm6ds3_context, LSM6DS3_1000dps);
	if (error) return error;
	error = lsm6ds3_xl_data_rate_set(lsm6ds3_context, LSM6DS3_XL_ODR_104Hz);
	if (error) return error;
	error = lsm6ds3_gy_data_rate_set(lsm6ds3_context, LSM6DS3_GY_ODR_104Hz);
	return error;
}


int32_t lsm6ds3_get_accel_data_mg(stmdev_ctx_t *lsm6ds3_context, float* accel)
{
	uint8_t data_raw [6];
	int32_t error;
	error = lsm6ds3_acceleration_raw_get(lsm6ds3_context, data_raw);
	if (error) return error;
	accel[0] = lsm6ds3_from_fs4g_to_mg((data_raw[1] << 8) | data_raw[0]);
	accel[1] = lsm6ds3_from_fs4g_to_mg((data_raw[3] << 8) | data_raw[2]);
	accel[2] = lsm6ds3_from_fs4g_to_mg((data_raw[5] << 8) | data_raw[4]);
	return 0;
}

