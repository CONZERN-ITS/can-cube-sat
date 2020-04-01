/*
 * time.c
 *
 *  Created on: 31 мар. 2020 г.
 *      Author: developer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/Trace.h"
#include "stm32f4xx_hal.h"


// Init timer for TOW in ms
void InitTowMsTimer(TIM_HandleTypeDef * htim)
{
	int error = 0;

	__TIM2_CLK_ENABLE();
	htim->Instance = TIM2;
	htim->Init.Prescaler = 0;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.Period = 604799999;	 			//миллисекунды в неделе

	TIM_SlaveConfigTypeDef sSlaveConfig;
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR3;	// slave - TIM2, master - TIM4
	sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	error = HAL_TIM_SlaveConfigSynchronization(htim, &sSlaveConfig);
	trace_printf("Slave Config Sync error %d\n", error);

	TIM_MasterConfigTypeDef sMasterConfig;
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig);

	error = 0;
	error = HAL_TIM_Base_Init(htim);
	trace_printf("TOW ms init error %d\n", error);


}

void InitMasterTimer(TIM_HandleTypeDef * htim)
{
	int error = 0;

	__TIM4_CLK_ENABLE();
	htim->Instance = TIM4;
	htim->Init.Prescaler = 41999;		// 2000 Hz
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.Period = 1;

	TIM_MasterConfigTypeDef sMasterConfig;
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	error = HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig);
	trace_printf("Master Config Sync error %d\n", error);

	error = 0;
	error = HAL_TIM_Base_Init(htim);
	trace_printf("Master timer init error %d\n", error);
}
