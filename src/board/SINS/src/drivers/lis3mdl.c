/*
 * lis3mdl.c
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#include <stdint.h>
#include <stm32f4xx.h>
#include <diag/Trace.h>

#include "lis3mdl_reg.h"
#include "state.h"

#include "lis3mdl.h"
#include "vector.h"

#define LSM_TIMEOUT	1000
#define MDPS_TO_RAD	M_PI / 180 / 1000

//	Magnetometer bias & transform matrix
#define X_MAGN_OFFSET		 1.000000
#define Y_MAGN_OFFSET		 1.000000
#define Z_MAGN_OFFSET		 1.000000
#define XX_MAGN_TRANSFORM_MATIX	 0.000000
#define YY_MAGN_TRANSFORM_MATIX	 0.000000
#define ZZ_MAGN_TRANSFORM_MATIX	 0.000000
#define XY_MAGN_TRANSFORM_MATIX	 0.000000
#define XZ_MAGN_TRANSFORM_MATIX	 0.000000
#define YZ_MAGN_TRANSFORM_MATIX	 0.000000


typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;


//static axis3bit16_t data_raw_magnetic;
//static axis1bit16_t data_raw_temperature;
//static float magnetic_mG[3];
//static float temperature_degC;
static uint8_t whoamI, rst;
//static uint8_t tx_buffer[1000];


stmdev_ctx_t lis3mdl_dev_ctx;

static int32_t lis3mdl_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t lis3mdl_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);


int lis3mdl_init(void)
{
	int error = 0;

	lis3mdl_dev_ctx.write_reg = lis3mdl_write;
	lis3mdl_dev_ctx.read_reg = lis3mdl_read;
	lis3mdl_dev_ctx.handle = &spi;

	// Reset to defaults
	error |= lis3mdl_reset_set(&lis3mdl_dev_ctx, PROPERTY_ENABLE);
	do {
		error = lis3mdl_reset_get(&lis3mdl_dev_ctx, &rst);
	} while (rst);

	// Check who_am_i
	error |= lis3mdl_device_id_get(&lis3mdl_dev_ctx, &whoamI);
	if (whoamI != LIS3MDL_ID)
	{
		trace_printf("lis3mdl not found, %d\terror: %d\n", whoamI, error);
		return -19;
	}
	else
		trace_printf("lis3mdl OK\n");

	error |= lis3mdl_block_data_update_set(&lis3mdl_dev_ctx, PROPERTY_DISABLE);

	error |= lis3mdl_data_rate_set(&lis3mdl_dev_ctx, LIS3MDL_HP_40Hz);

	error |= lis3mdl_full_scale_set(&lis3mdl_dev_ctx, LIS3MDL_16_GAUSS);

	error |= lis3mdl_operating_mode_set(&lis3mdl_dev_ctx, LIS3MDL_CONTINUOUS_MODE);

	return error;
}


uint32_t lis3mdl_get_m_data_mG(float* magn)
{
	axis3bit16_t data_raw_magnetic;
	uint8_t error;
	//	Read data
	PROCESS_ERROR(lis3mdl_magnetic_raw_get(&lis3mdl_dev_ctx, data_raw_magnetic.u8bit));
	magn[0] = 1000 * LIS3MDL_FROM_FS_16G_TO_G(data_raw_magnetic.i16bit[0]);
	magn[1] = 1000 * LIS3MDL_FROM_FS_16G_TO_G(data_raw_magnetic.i16bit[1]);
	magn[2] = 1000 * LIS3MDL_FROM_FS_16G_TO_G(data_raw_magnetic.i16bit[2]);

	if (!CALIBRATION)
	{
		//	Magnetometer bias and transform matrix (to provide real values)
		float offset_vector[3] = {X_MAGN_OFFSET, Y_MAGN_OFFSET, Z_MAGN_OFFSET};
		float transform_matrix[3][3] =	{	{XX_MAGN_TRANSFORM_MATIX, XY_MAGN_TRANSFORM_MATIX, XZ_MAGN_TRANSFORM_MATIX},
											{XY_MAGN_TRANSFORM_MATIX, YY_MAGN_TRANSFORM_MATIX, YZ_MAGN_TRANSFORM_MATIX},
											{XZ_MAGN_TRANSFORM_MATIX, YZ_MAGN_TRANSFORM_MATIX, ZZ_MAGN_TRANSFORM_MATIX}};

		vmv(magn, offset_vector, magn);
		mxv(transform_matrix, magn, magn);

		//	Change axes to be like in accelerometer FIXME: посмотреть в datasheet направление осей
		float tmp = magn[0];
		magn[0] = -magn[1];
		magn[1] = -tmp;
	}

end:
	return error;
}


static int32_t lis3mdl_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
	int error = 0;

	if (handle == &spi)
	{
		/* Write multiple command */
		reg |= 0x40;
		HAL_GPIO_WritePin(PORT, CS_PIN_MAGN, GPIO_PIN_RESET);
		HAL_SPI_Transmit(handle, &reg, 1, 1000);
		HAL_SPI_Transmit(handle, bufp, len, 1000);
		HAL_GPIO_WritePin(PORT, CS_PIN_MAGN, GPIO_PIN_SET);
	}
	else
		{
			trace_printf("lis3mdl invalid handle\n");
			error = -19;
		}
	return error;
}


static int32_t lis3mdl_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
	int error = 0;

	if (handle == &spi)
	{
		/* Read multiple command */
		reg |= 0xC0;
		HAL_GPIO_WritePin(PORT, CS_PIN_MAGN, GPIO_PIN_RESET);
		HAL_SPI_Transmit(handle, &reg, 1, 1000);
		HAL_SPI_Receive(handle, bufp, len, 1000);
		HAL_GPIO_WritePin(PORT, CS_PIN_MAGN, GPIO_PIN_SET);
	}
	else
	{
		trace_printf("lis3mdl invalid handle\n");
		error = -19;
	}

	return error;
}

