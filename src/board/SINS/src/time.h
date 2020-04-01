/*
 * time.h
 *
 *  Created on: 31 мар. 2020 г.
 *      Author: developer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/Trace.h"
#include "stm32f4xx_hal.h"

#ifndef TIME_H_
#define TIME_H_

extern void InitTowMsTimer(TIM_HandleTypeDef * htim);
extern void InitMasterTimer(TIM_HandleTypeDef * htim);


#endif /* TIME_H_ */
