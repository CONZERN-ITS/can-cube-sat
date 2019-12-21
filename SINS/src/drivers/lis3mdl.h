/*
 * lis3mdl.h
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: developer
 */

#ifndef DRIVERS_LIS3MDL_H_
#define DRIVERS_LIS3MDL_H_

#include "../library/lis3mdl_STdC/driver/lis3mdl_reg.h"

SPI_HandleTypeDef spi;

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw_magnetic;
static axis1bit16_t data_raw_temperature;
static float magnetic_mG[3];
static float temperature_degC;
static uint8_t whoamI, rst;
static uint8_t tx_buffer[1000];

extern stmdev_ctx_t lis3mdl_dev_ctx;


/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com(uint8_t *tx_buffer, uint16_t len);
static void platform_init(void);


#endif /* DRIVERS_LIS3MDL_H_ */
