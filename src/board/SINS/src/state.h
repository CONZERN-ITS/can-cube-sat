/*
 * state.h
 *
 *  Created on: 20 дек. 2019 г.
 *      Author: developer
 */

#include "stm32f4xx_hal.h"
#include "drivers/lis3mdl.h"
#include "drivers/lsm6ds3.h"
#include "drivers/gps/gps.h"
#include "time.h"

#ifndef STATE_H_
#define STATE_H_


//--------- SPI ---------------
#define PORT				GPIOA
#define CS_PIN_ACCEL		GPIO_PIN_4
#define CS_PIN_MAGN			GPIO_PIN_0
#define SCK_PIN				GPIO_PIN_5
#define MISO_PIN			GPIO_PIN_6
#define MOSI_PIN			GPIO_PIN_7


#define CALIBRATION 0
#define GPS			0
#define LSM6DS3		0
#define LIS3MDL		0

// if error set value and go to end
#define PROCESS_ERROR(x) if (0 != (error = (x))) { goto end; }


typedef struct {
	uint8_t bus_state;
	uint8_t lis3mdl_state;
	uint8_t lsm6ds3_state;
	uint8_t GPS_state;
	uint32_t time;
}state_system_t;


typedef struct {
	float accel[3];
	float gyro[3];
	float magn[3];
}stateSINS_rsc_t;


typedef struct {
	//	zero params; this fields should be filled when device started it`s work
	float zero_quaternion[4];
	float gyro_staticShift[3];
	float accel_staticShift[3];
	float zero_GPS[3];
} state_zero_t;


typedef struct {
	float quaternion[4];
	float accel[3];
	float magn[3];
}stateSINS_isc_t;


typedef struct {
	float time;
	float coordinates[3];

}stateGPS_t;

typedef struct {
	float quaternion[4];

}__attribute__((packed, aligned(1))) stateSINS_transfer_t;

extern SPI_HandleTypeDef spi;
extern I2C_HandleTypeDef i2c;

extern UART_HandleTypeDef uartTransfer_data;
extern UART_HandleTypeDef uartGPS;


extern stmdev_ctx_t lsm6ds3_dev_ctx;
extern stmdev_ctx_t lis3mdl_dev_ctx;
extern stateSINS_rsc_t stateSINS_rsc;
extern state_system_t state_system;
extern state_zero_t state_zero;
extern state_system_t state_system_prev;
extern stateSINS_isc_t stateSINS_isc;
extern stateSINS_isc_t stateSINS_isc_prev;
extern stateSINS_transfer_t stateSINS_transfer;
extern stateGPS_t stateGPS;


extern int32_t bus_spi_init(void * handle);
extern void init_led(void);
//void SENSORS_Init(void);
extern void SensorsInit(void);
extern int UpdateDataAll(void);
extern void SINS_updatePrevData(void);

#endif /* STATE_H_ */
