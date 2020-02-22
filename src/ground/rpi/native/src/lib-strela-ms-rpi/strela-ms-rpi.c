/*
 * strela-ms-rpi.c
 *
 *  Created on: Feb 21, 2020
 *      Author: developer
 */
#include "lis3mdl_reg.h"
#include "lsm6ds3_reg.h"

typedef enum {
	ERROR_NO_ERROR = 0,
	ERROR_UNKNOWN_ID = -1
} error_t;

stmdev_ctx_t lis3mdl_context;
stmdev_ctx_t lsm6ds3_context;

int32_t lis3mdl_init (stmdev_write_ptr read, stmdev_write_ptr write){
	int32_t error = 0;
	uint8_t reset = 1;
	uint8_t id = 0;

	// Interface context initialization
	lis3mdl_context.write_reg = write;
	lis3mdl_context.read_reg = read;
	lis3mdl_context.handle = 0;

	// Reset
	error = lis3mdl_reset_set(&lis3mdl_context, PROPERTY_ENABLE);
	if (error) return error;
	while (reset){
		error = lis3mdl_reset_get(&lis3mdl_context, &reset);
		if (error) return error;
	}

	// Check id
	error = lis3mdl_device_id_get(&lis3mdl_context, &id);
	if (error) return error;
	if (id != LIS3MDL_ID) return ERROR_UNKNOWN_ID;

	// Set configuration
	error = lis3mdl_block_data_update_set(&lis3mdl_context, PROPERTY_DISABLE);
	if (error) return error;
	error = lis3mdl_data_rate_set(&lis3mdl_context, LIS3MDL_HP_40Hz);
	if (error) return error;
	error = lis3mdl_full_scale_set(&lis3mdl_context, LIS3MDL_16_GAUSS);
	if (error) return error;
	error = lis3mdl_operating_mode_set(&lis3mdl_context, LIS3MDL_CONTINUOUS_MODE);
	return error;
}

int32_t lis3mdl_get_data_G (float* data){
	uint8_t data_raw [6];
	uint8_t error;

	error = lis3mdl_magnetic_raw_get(&lis3mdl_context, data_raw);
	if (error) return error;
	data[0] = LIS3MDL_FROM_FS_16G_TO_G((data_raw[1] << 8) | data_raw[0]);
	data[1] = LIS3MDL_FROM_FS_16G_TO_G((data_raw[3] << 8) | data_raw[2]);
	data[2] = LIS3MDL_FROM_FS_16G_TO_G((data_raw[5] << 8) | data_raw[4]);
	return 0;
}

int32_t lsm6ds3_init(stmdev_write_ptr read, stmdev_write_ptr write){
	int32_t error = 0;
	uint8_t reset = 1;
	uint8_t id = 0;

	// Interface context initialization
	lsm6ds3_context.write_reg = write;
	lsm6ds3_context.read_reg = read;
	lsm6ds3_context.handle = 0;

	// Reset
	error = lsm6ds3_reset_set(&lsm6ds3_context, PROPERTY_ENABLE);
	if (error) return error;
	while (reset){
		error = lsm6ds3_reset_get(&lsm6ds3_context, &reset);
		if (error) return error;
	}

	// Check id
	error = lsm6ds3_device_id_get(&lsm6ds3_context, &id);
	if (error) return error;
	if (id != LSM6DS3_ID) return ERROR_UNKNOWN_ID;

	// Set configuration
	error = lsm6ds3_fifo_mode_set(&lsm6ds3_context, PROPERTY_DISABLE);
	if (error) return error;
	error = lsm6ds3_block_data_update_set(&lsm6ds3_context, PROPERTY_DISABLE);
	if (error) return error;
	error = lsm6ds3_xl_full_scale_set(&lsm6ds3_context, LSM6DS3_4g);
	if (error) return error;
	error = lsm6ds3_gy_full_scale_set(&lsm6ds3_context, LSM6DS3_1000dps);
	if (error) return error;
	error = lsm6ds3_xl_data_rate_set(&lsm6ds3_context, LSM6DS3_XL_ODR_104Hz);
	if (error) return error;
	error = lsm6ds3_gy_data_rate_set(&lsm6ds3_context, LSM6DS3_GY_ODR_104Hz);
	if (error) return error;
	error = lsm6ds3_xl_filter_analog_set(&lsm6ds3_context, LSM6DS3_ANTI_ALIASING_200Hz);
	return error;
}

int32_t lsm6ds3_get_accel_data_mg(float* accel)
{
	uint8_t data_raw [6];
	int32_t error;

	error = lsm6ds3_acceleration_raw_get(&lsm6ds3_context, data_raw);
	if (error) return error;
	accel[0] = lsm6ds3_from_fs4g_to_mg((data_raw[1] << 8) | data_raw[0]);
	accel[1] = lsm6ds3_from_fs4g_to_mg((data_raw[3] << 8) | data_raw[2]);
	accel[2] = lsm6ds3_from_fs4g_to_mg((data_raw[5] << 8) | data_raw[4]);
	return 0;
}

