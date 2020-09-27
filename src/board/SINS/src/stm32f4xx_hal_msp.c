/**
  ******************************************************************************
  * @file    stm32f4xx_hal_msp_template.c
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    06-May-2016
  * @brief   This file contains the HAL System and Peripheral (PPP) MSP initialization
  *          and de-initialization functions.
  *          It should be copied to the application folder and renamed into 'stm32f4xx_hal_msp.c'.           
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "sins_config.h"

// [ILG]
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP HAL MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions HAL MSP Private Functions
  * @{
  */

/**
  * @brief  Initializes the Global MSP.
  * @note   This function is called from HAL_Init() function to perform system
  *         level initialization (GPIOs, clock, DMA, interrupt).
  * @retval None
  */
void HAL_MspInit(void)
{
//	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

/**
  * @brief  DeInitializes the Global MSP.
  * @note   This functiona is called from HAL_DeInit() function to perform system
  *         level de-initialization (GPIOs, clock, DMA, interrupt).
  * @retval None
  */
void HAL_MspDeInit(void)
{

}


void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{

	if(hi2c->Instance == I2C2)
	{
		__I2C2_CLK_ENABLE();
		__GPIOB_CLK_ENABLE();

		GPIO_InitTypeDef gpiob;
		gpiob.Alternate = GPIO_AF4_I2C2;
		gpiob.Mode = GPIO_MODE_AF_OD;
		gpiob.Pin = GPIO_PIN_10 | GPIO_PIN_11;		// SCL/SDA
		gpiob.Pull = GPIO_NOPULL;
		gpiob.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOB, &gpiob);
	}
}


void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi){
	if (hspi->Instance == SPI1) {
		__SPI1_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		__GPIOC_CLK_ENABLE();


		GPIO_InitTypeDef gpioa;
		gpioa.Alternate = GPIO_AF5_SPI1;
		gpioa.Mode = GPIO_MODE_AF_PP;
		gpioa.Pin = GPIO_PIN_5 | GPIO_PIN_7;
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &gpioa);

		gpioa.Mode = GPIO_MODE_OUTPUT_PP;
		gpioa.Pin = GPIO_PIN_4 | GPIO_PIN_0;
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &gpioa);

		gpioa.Mode = GPIO_MODE_AF_PP;
		gpioa.Pin = GPIO_PIN_6;
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_GPIO_Init(GPIOA, &gpioa);


		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_0, GPIO_PIN_SET);

//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, SET);
	}
}


void HAL_UART_MspInit(UART_HandleTypeDef *huart){

	//	transmit uart
	if (huart->Instance == USART1) {
		__USART1_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();

		GPIO_InitTypeDef gpioa;
		gpioa.Alternate = GPIO_AF7_USART1;
		gpioa.Mode = GPIO_MODE_AF_PP;
		gpioa.Pin = GPIO_PIN_9;		//TX		//FIXME: вернуть PA9
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &gpioa);

		gpioa.Alternate = GPIO_AF7_USART1;
		gpioa.Mode = GPIO_MODE_AF_OD;
		gpioa.Pin = GPIO_PIN_10;		//RX		//FIXME: вернуть PA10
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &gpioa);
	}

	//	uart_GPS
	if(huart->Instance == USART2) {
		__USART2_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();

		GPIO_InitTypeDef gpioa;
		gpioa.Alternate = GPIO_AF7_USART2;
		gpioa.Mode = GPIO_MODE_AF_PP;
		gpioa.Pin = GPIO_PIN_2;		//TX
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &gpioa);

		gpioa.Alternate = GPIO_AF7_USART2;
		gpioa.Mode = GPIO_MODE_AF_OD;
		gpioa.Pin = GPIO_PIN_3;		//RX
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &gpioa);

		HAL_NVIC_SetPriority(USART2_IRQn, ITS_SINS_GPS_UART_IRQ_PRIORITY, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}

	if (huart->Instance == USART3) {
		__USART3_CLK_ENABLE();
		__GPIOC_CLK_ENABLE();

		GPIO_InitTypeDef gpioa;
		gpioa.Alternate = GPIO_AF7_USART3;
		gpioa.Mode = GPIO_MODE_AF_PP;
		gpioa.Pin = GPIO_PIN_10;		//TX
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOC, &gpioa);

		gpioa.Alternate = GPIO_AF7_USART3;
		gpioa.Mode = GPIO_MODE_AF_OD;
		gpioa.Pin = GPIO_PIN_11;		//RX
		gpioa.Pull = GPIO_NOPULL;
		gpioa.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOC, &gpioa);
	}
}


/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(htim_base->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM2_IRQn, ITS_SINS_TIME_SVC_TOW_OVF_PRIOIRITY, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
	}
	else if(htim_base->Instance==TIM3)
	{
		/* Peripheral clock enable */
		__HAL_RCC_TIM3_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		/**TIM3 GPIO Configuration
		PA7     ------> TIM3_CH2 для TRGI на сброс по PPS сигналу
		PD2     ------> TIM3_ETR для входной частоты от TIM4
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	}
	else if(htim_base->Instance==TIM4)
	{
		/* Peripheral clock enable */
		__HAL_RCC_TIM4_CLK_ENABLE();

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**TIM4 GPIO Configuration
		PB7     ------> TIM4_CH2 для TRGI на сброс по PPS сигналу
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
	else if(htim_base->Instance==TIM6)
	{
		/* Peripheral clock enable */
		__HAL_RCC_TIM6_CLK_ENABLE();

		/* TIM6 interrupt Init */
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, ITS_SINS_TIME_SVC_STEADY_OVF_PRIORITY, 0);
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	}
	else if(htim_base->Instance == TIM1)
	{
	    __HAL_RCC_TIM1_CLK_ENABLE();
	}
}


void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(htim->Instance==TIM3)
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**TIM3 GPIO Configuration
		PA6     ------> TIM3_CH1 для передачи псвердо PPS импульсов для остальных вычислителей системы
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else if(htim->Instance==TIM4)
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**TIM4 GPIO Configuration
		PB6     ------> TIM4_CH1 // для питания TIM3 опорной частотой
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}


/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance==TIM2)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM2_CLK_DISABLE();
	}
	else if(htim_base->Instance==TIM3)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM3_CLK_DISABLE();

		/**TIM3 GPIO Configuration
		PA6     ------> TIM3_CH1
		PA7     ------> TIM3_CH2
		PD2     ------> TIM3_ETR
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6|GPIO_PIN_7);

		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
	}
	else if(htim_base->Instance==TIM4)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM4_CLK_DISABLE();

		/**TIM4 GPIO Configuration
		PB6     ------> TIM4_CH1
		PB7     ------> TIM4_CH2
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
	}
}


/**
* @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
	if(hrtc->Instance==RTC)
	{
		__HAL_RCC_RTC_ENABLE();

		HAL_NVIC_SetPriority(RTC_Alarm_IRQn, ITS_SINS_TIME_SVC_ALARM_IRQ_PRIORITY, 0);
		HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	}
}


//! Не совсем халовский MSP, но пускай уж будет тут как остальные настройки нвика
void NONHAL_PPS_MspInit(void)
{
	HAL_NVIC_SetPriority(EXTI0_IRQn, ITS_SINS_GPS_PPS_IRQ_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();
  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}


void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{

	if(hi2c->Instance == I2C2)
	{
		__I2C2_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
	}
}


/**
  * @brief  Initializes the PPP MSP.
  * @note   This functiona is called from HAL_PPP_Init() function to perform 
  *         peripheral(PPP) system level initialization (GPIOs, clock, DMA, interrupt)
  * @retval None
  */
void HAL_PPP_MspInit(void)
{

}

/**
  * @brief  DeInitializes the PPP MSP.
  * @note   This functiona is called from HAL_PPP_DeInit() function to perform 
  *         peripheral(PPP) system level de-initialization (GPIOs, clock, DMA, interrupt)
  * @retval None
  */
void HAL_PPP_MspDeInit(void)
{

}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

// [ILG]
#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
