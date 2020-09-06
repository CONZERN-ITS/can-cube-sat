/*
 * errors.h
 *
 *  Created on: 19 авг. 2020 г.
 *      Author: developer
 */

#ifndef ERRORS_H_
#define ERRORS_H_

typedef enum
{
	berr = I2C_FLAG_BERR,
	arlo = HAL_I2C_ERROR_ARLO,
	af = HAL_I2C_ERROR_AF,
	ovr = HAL_I2C_ERROR_OVR,
	dma_error = HAL_I2C_ERROR_DMA,
	timeout = HAL_I2C_ERROR_TIMEOUT,

} i2c_error_codes;


//extern void i2c_error_handler(I2C_HandleTypeDef *hi2c);
void error_system_check(void);
void error_mems_read(void);


#endif /* ERRORS_H_ */
