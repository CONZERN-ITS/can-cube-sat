/*
 * lis3mdl.c
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#include "lis3mdl.h"


#include <stdint.h>
#include <stm32f4xx.h>
#include <diag/Trace.h>

#include "../library/lis3mdl_STdC/driver/lis3mdl_reg.h"
//#include "state.h"
//#include "vector.h"


#define LSM_TIMEOUT	1000
#define MDPS_TO_RAD	M_PI / 180 / 1000

//	Magnetometer bias & transform matrix
#define X_MAGN_OFFSET		-292.920973
#define Y_MAGN_OFFSET		 224.742275
#define Z_MAGN_OFFSET		-130.841676
#define XX_MAGN_TRANSFORM_MATIX	 0.002098
#define YY_MAGN_TRANSFORM_MATIX	 0.002186
#define ZZ_MAGN_TRANSFORM_MATIX	 0.002267
#define XY_MAGN_TRANSFORM_MATIX	 0.000118
#define XZ_MAGN_TRANSFORM_MATIX	-0.000354
#define YZ_MAGN_TRANSFORM_MATIX	 0.000003


static uint8_t whoamI, rst;


stmdev_ctx_t lis3mdl_dev_ctx;

static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
	int error = 0;

	if (handle == spi){
		reg |= 0x40;
	    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
	    HAL_SPI_Transmit(handle, &reg, 1, 1000);
	    HAL_SPI_Transmit(handle, bufp, len, 1000);
	    HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
	}

	else
	{
		trace_printf("lsm303c invalid handle\n");
		error = -19;
	}

	return error;
}

