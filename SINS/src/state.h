/*
 * state.h
 *
 *  Created on: 20 дек. 2019 г.
 *      Author: developer
 */

#include "stm32f4xx_hal.h"
#include "drivers/lis3mdl.h"
#include "drivers/lsm6ds3.h"

#ifndef STATE_H_
#define STATE_H_


//--------- LIS3MDL ---------------
#define PORT		GPIOA
#define CS_PIN		GPIO_PIN_4
#define SCK_PIN		GPIO_PIN_5
#define MISO_PIN	GPIO_PIN_6
#define MOSI_PIN	GPIO_PIN_7


#define CALIBRATION 0


// if error set value and go to end
#define PROCESS_ERROR(x) if (0 != (error = (x))) { goto end; }

extern SPI_HandleTypeDef spi;
extern stmdev_ctx_t lsm6ds3_dev_ctx;
extern stmdev_ctx_t lis3mdl_dev_ctx;

int32_t bus_init(void * handle);
void init_led(void);


#endif /* STATE_H_ */
