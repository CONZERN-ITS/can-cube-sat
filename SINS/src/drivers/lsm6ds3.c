/*
 * lsm6ds3.c
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#include "lsm6ds3.h"
#include "lsm6ds3_reg.h"
#include "state.h"
#include "vector.h"
#include "diag/Trace.h"


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

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;


stmdev_ctx_t lsm6ds3_dev_ctx;

static uint8_t whoamI, rst;


static int32_t lsm6ds3_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lsm6ds3_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);


int32_t lsm6ds3_init(void)
{
	int error = 0;

	lsm6ds3_dev_ctx.write_reg = lsm6ds3_write;
	lsm6ds3_dev_ctx.read_reg = lsm6ds3_read;
	lsm6ds3_dev_ctx.handle = &spi;

	// Reset to defaults
	error |= lsm6ds3_reset_set(&lsm6ds3_dev_ctx, PROPERTY_ENABLE);
	do {
		error = lsm6ds3_reset_get(&lsm6ds3_dev_ctx, &rst);
	} while (rst);

	// Check who_am_i
	error |= lsm6ds3_device_id_get(&lsm6ds3_dev_ctx, &whoamI);
	if (whoamI != LSM6DS3_ID)
	{
		trace_printf("lsm6ds3 not found, %d\terror: %d\n", whoamI, error);
		return -19;
	}
	else
		trace_printf("lsm6ds3 OK\n");

	error |= lsm6ds3_fifo_mode_set(&lsm6ds3_dev_ctx, PROPERTY_DISABLE);

	error |= lsm6ds3_block_data_update_set(&lsm6ds3_dev_ctx, PROPERTY_DISABLE);

	error |= lsm6ds3_xl_full_scale_set(&lsm6ds3_dev_ctx, LSM6DS3_4g);
	error |= lsm6ds3_gy_full_scale_set(&lsm6ds3_dev_ctx, LSM6DS3_1000dps);

	error |= lsm6ds3_xl_data_rate_set(&lsm6ds3_dev_ctx, LSM6DS3_XL_ODR_104Hz);
	error |= lsm6ds3_gy_data_rate_set(&lsm6ds3_dev_ctx, LSM6DS3_GY_ODR_104Hz);

	error |= lsm6ds3_xl_filter_analog_set(&lsm6ds3_dev_ctx, LSM6DS3_ANTI_ALIASING_200Hz);

	return error;
}


uint32_t lsm6ds3_get_xl_data_g(float* accel)
{
	axis3bit16_t data_raw_acceleration;
	uint8_t error;
	//	Read acceleration field data
	PROCESS_ERROR(lsm6ds3_acceleration_raw_get(&lsm6ds3_dev_ctx, data_raw_acceleration.u8bit));
	accel[0] = lsm6ds3_from_fs4g_to_mg(data_raw_acceleration.i16bit[0]) * MG_TO_MPS2;
	accel[1] = lsm6ds3_from_fs4g_to_mg(data_raw_acceleration.i16bit[1]) * MG_TO_MPS2;
	accel[2] = lsm6ds3_from_fs4g_to_mg(data_raw_acceleration.i16bit[2]) * MG_TO_MPS2;

	if (!CALIBRATION)
	{
		//	Accelerometer bias and transform matrix (to provide real values)
		float offset_vector[3] = {X_ACCEL_OFFSET, Y_ACCEL_OFFSET, Z_ACCEL_OFFSET};
		float transform_matrix[3][3] =	{{XX_ACCEL_TRANSFORM_MATIX, XY_ACCEL_TRANSFORM_MATIX, XZ_ACCEL_TRANSFORM_MATIX},
										 {XY_ACCEL_TRANSFORM_MATIX, YY_ACCEL_TRANSFORM_MATIX, YZ_ACCEL_TRANSFORM_MATIX},
										 {XZ_ACCEL_TRANSFORM_MATIX, YZ_ACCEL_TRANSFORM_MATIX, ZZ_ACCEL_TRANSFORM_MATIX}};

		vmv(accel, offset_vector, accel);
		mxv(transform_matrix, accel, accel);
	}

end:
	return error;
}


uint32_t lsm6ds3_get_g_data_rps(float* gyro)
{
	axis3bit16_t data_raw_angular_rate;
	uint8_t error;
	//	Read acceleration field data
	error = lsm6ds3_angular_rate_raw_get(&lsm6ds3_dev_ctx, data_raw_angular_rate.u8bit);
	gyro[0] = lsm6ds3_from_fs1000dps_to_mdps(data_raw_angular_rate.i16bit[0]) * MDPS_TO_RAD;
	gyro[1] = lsm6ds3_from_fs1000dps_to_mdps(data_raw_angular_rate.i16bit[1]) * MDPS_TO_RAD;
	gyro[2] = lsm6ds3_from_fs1000dps_to_mdps(data_raw_angular_rate.i16bit[2]) * MDPS_TO_RAD;
	return error;
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

	if (handle == &spi)
	{
		uint32_t error = 0;
		HAL_GPIO_WritePin(PORT, CS_PIN, GPIO_PIN_RESET);
		error |= HAL_SPI_Transmit(handle, &reg, 1, 1000);
		error |= HAL_SPI_Transmit(handle, bufp, len, 1000);
		HAL_GPIO_WritePin(PORT, CS_PIN, GPIO_PIN_SET);

		return error;
	}
	trace_printf("lsm6ds3 invalid handle\n");
	return -19;
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

	if (handle == &spi)
	{
		uint32_t error = 0;
		/* Read command */
		reg |= 0x80;
		HAL_GPIO_WritePin(PORT, CS_PIN, GPIO_PIN_RESET);
		error |= HAL_SPI_Transmit(handle, &reg, 1, 1000);
		error |= HAL_SPI_Receive(handle, bufp, len, 1000);
		HAL_GPIO_WritePin(PORT, CS_PIN, GPIO_PIN_SET);

		return error;
	}
	trace_printf("lsm6ds3 invalid handle\n");
	return -19;
}

