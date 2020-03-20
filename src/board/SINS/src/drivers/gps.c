/*
 * gps.c
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#include <stdint.h>
#include <stm32f4xx.h>
#include <diag/Trace.h>


#include "state.h"
#include "gps.h"



void initInterruptPin()
{
	__GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpioa;
	gpioa.Mode = GPIO_MODE_IT_RISING;
	gpioa.Pin = GPIO_PIN_0;
	gpioa.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpioa);

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	trace_printf("EXTI0 IRQ enabled\n");
//	EXTI->RTSR |= (1 << 0);

}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	if (GPIO_Pin == GPIO_PIN_0)
//	{
//		trace_printf("1");
//	}
//}

void EXTI0_IRQHandler()
{
//	trace_printf("1");

	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}





