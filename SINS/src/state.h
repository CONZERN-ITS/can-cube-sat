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


typedef struct {
	uint8_t bus_state;
	uint8_t lis3mdl_state;
	uint8_t lsm6ds3_state;
	uint32_t time;
}state_system_t;


typedef struct {
	float accel[3];
	float gyro[3];
	float magn[3];
}stateSINS_rsc_t;


typedef struct {
	float gyro_staticShift[3];
	float accel_staticShift[3];
}state_zero_t;


typedef struct {
	float quaternion[4];
	float accel[3];
	float gyro[3];
	float magn[3];
}stateSINS_isc_t;


extern SPI_HandleTypeDef spi;

extern stmdev_ctx_t lsm6ds3_dev_ctx;
extern stmdev_ctx_t lis3mdl_dev_ctx;
extern stateSINS_rsc_t stateSINS_rsc;
extern state_system_t state_system;
extern state_zero_t state_zero;
extern state_system_t state_system_prev;
extern stateSINS_isc_t stateSINS_isc;
extern stateSINS_isc_t stateSINS_isc_prev;


int32_t bus_init(void * handle);
void init_led(void);
void SENSORS_Init(void);
int UpdateDataAll(void);
void SINS_updatePrevData(void);

#endif /* STATE_H_ */
