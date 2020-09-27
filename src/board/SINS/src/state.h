/*
 * state.h
 *
 *  Created on: 20 дек. 2019 г.
 *      Author: developer
 */

#include <time.h>

#include "stm32f4xx_hal.h"
#include "drivers/mems/mems.h"
#include "drivers/gps/gps.h"


#ifndef STATE_H_
#define STATE_H_


#define CALIBRATION 	0
#define CALIBRATION_LSM 0
#define CALIBRATION_LIS 0
#define GPS				0
#define LSM6DS3			0
#define LIS3MDL			0

// if error set value and go to end
#define PROCESS_ERROR(x) if (0 != (error = (x))) { goto end; }


typedef struct {
	int gps_init_error;
	int gps_config_error;
	int lsm6ds3_error;
	int lis3mdl_error;
	int analog_sensor_init_error;

	int gps_uart_init_error;
	int gps_uart_error;

	int mems_i2c_error;

	int uart_transfer_init_error;
	int uart_transfer_error;

	int timers_error;
	int rtc_error;

	int mems_i2c_error_counter;
	int lsm6ds3_error_counter;
	int lis3mdl_error_counter;

	int gps_reconfig_counter;

	uint8_t reset_counter;
}error_system_t;


typedef struct {
	uint8_t bus_state;
	uint8_t lis3mdl_state;
	uint8_t lsm6ds3_state;
	uint8_t GPS_state;
	struct timeval tv;
}state_system_t;


typedef struct {
	struct timeval tv;
	float accel[3];
	float gyro[3];
	float magn[3];
}stateSINS_rsc_t;


typedef struct {
	//	zero params; this fields should be filled when device started it`s work
	struct timeval tv;
	float zero_quaternion[4];
	float gyro_staticShift[3];
	float accel_staticShift[3];
	float zero_GPS[3];
} state_zero_t;


typedef struct {
	struct timeval tv;
	float accel[3];
	float magn[3];
	float quaternion[4];
}stateSINS_isc_t;


//typedef struct {
//	float time;
//	float coordinates[3];
//
//}stateGPS_t;
//
//typedef struct {
//	float quaternion[4];
//
//}__attribute__((packed, aligned(1))) stateSINS_transfer_t;


extern error_system_t error_system;
extern stateSINS_rsc_t stateSINS_rsc;
extern state_system_t state_system;
extern state_zero_t state_zero;
extern state_system_t state_system_prev;
extern stateSINS_isc_t stateSINS_isc;
extern stateSINS_isc_t stateSINS_isc_prev;
//extern stateSINS_transfer_t stateSINS_transfer;
//extern stateGPS_t stateGPS;


//void SENSORS_Init(void);
extern void SensorsInit(void);
extern int UpdateDataAll(void);
extern void SINS_updatePrevData(void);
extern int check_SINS_state(void);

#endif /* STATE_H_ */
