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

TIM_ClockConfigTypeDef sClockSourceConfig;



// Init timer for TOW in ms
void InitTowMsTimer(TIM_HandleTypeDef * htim)
{

//	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//	HAL_TIM_ConfigClockSource(htim3, &sClockSourceConfig);

	__TIM5_CLK_ENABLE();
	htim->Instance = TIM5;
	htim->Init.Prescaler = 0;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.Period = 604799999;
//	htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;


//	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
//	sSlaveConfig.InputTrigger = TIM_TS_ITR2;
//	sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV2;
//	HAL_TIM_SlaveConfigSynchronization(htim2, &sSlaveConfig);


//	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
//	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//	HAL_TIMEx_Ma(htim2, &sMasterConfig);

}


void InitMasterTimer(TIM_HandleTypeDef * htim)
{
	__TIM2_CLK_ENABLE();
	htim->Instance = TIM2;
	htim->Init.Prescaler = 41999;
	htim->Init.CounterMode = TIM_COUNTERMODE_UP;
	htim->Init.Period = 1;			//1209600000
	htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

}

//ну там прескейлер, период настриваем по вкусу
//Я поставил чтобы его выходом на триигерную линию был UPDATE эвент
//sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
//там можно разные штуки ставить, но пока попробуем так
//теперь тим2
//смотрим эту табличку в рефмане
//тут получается, что если я хочу запитать таймер 2 от триггера таймера 3, то мне нужно брать ITR2
//Запомнили
//Настриваем


