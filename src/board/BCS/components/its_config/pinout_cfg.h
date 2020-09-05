/*
 * pinout_cfg.h
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef MAIN_PINOUT_CFG_H_
#define MAIN_PINOUT_CFG_H_

#include "init_helper.h"

#ifdef ITS_ESP_DEBUG
#define ITS_PIN_I2CTM_SCL 	18//16
#define ITS_PIN_I2CTM_SDA 	19//17
#define ITS_PIN_I2C_INT 	5//18
#define ITS_PIN_TIME		21//19

#define ITS_PIN_UARTE_RX	22
#define ITS_PIN_UARTE_TX	23
#define ITS_PIN_UARTE_INT	25

#define ITS_PIN_UARTR_RX	26
#define ITS_PIN_UARTR_TX	27

#if !ITS_WIFI_SERVER
#define ITS_PIN_SPISR_SCK	25
#define ITS_PIN_SPISR_SS	26
#define ITS_PIN_SPISR_MOSI	27
#define ITS_PIN_SPISR_MISO	-1
#endif
#else

#define ITS_PIN_I2CTM_SCL 	16
#define ITS_PIN_I2CTM_SDA 	17
#if ITS_WIFI_SERVER
#define ITS_PIN_I2C_INT 	18
#else
#define ITS_PIN_I2C_INT 	5
#endif

#define ITS_PIN_TIME		19

#define ITS_PIN_UARTE_RX	22
#define ITS_PIN_UARTE_TX	23
#define ITS_PIN_UARTE_INT	39

#define ITS_PIN_UARTR_RX	35
#define ITS_PIN_UARTR_TX	32
#define ITS_PIN_M1			34

#define ITS_PIN_SPISR_SCK	25
#define ITS_PIN_SPISR_SS	26
#define ITS_PIN_SPISR_MOSI	27
#define ITS_PIN_SPISR_MISO	-1

#if ITS_WIFI_SERVER
#define ITS_PIN_LED			5
#else
#define ITS_PIN_LED			18
#endif

#define ITS_PIN_OWB			21

#define ITS_PIN_PL_VCC		33

#endif

#endif /* MAIN_PINOUT_CFG_H_ */
