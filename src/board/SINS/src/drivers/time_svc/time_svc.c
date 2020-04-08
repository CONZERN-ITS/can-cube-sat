/*
 * time_svc.c
 *
 *  Created on: 7 апр. 2020 г.
 *      Author: snork
 */

#include <stm32f4xx_hal.h>

#include <assert.h>


static TIM_HandleTypeDef htim2;
static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim4;
static RTC_HandleTypeDef hrtc;


static uint32_t _gps_week;


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


static void MX_TIM2_Init(void)
{
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	assert(HAL_TIM_Base_Init(&htim2) == HAL_OK);
	assert(HAL_TIM_PWM_Init(&htim2) == HAL_OK);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR3;
	assert(HAL_TIM_SlaveConfigSynchronization(&htim2, &sSlaveConfig) == HAL_OK);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	assert(HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) == HAL_OK);

	HAL_TIM_MspPostInit(&htim2);
}


static void MX_TIM3_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	assert(HAL_TIM_Base_Init(&htim3) == HAL_OK);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	sClockSourceConfig.ClockFilter = 0;
	assert(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) == HAL_OK);
	assert(HAL_TIM_PWM_Init(&htim3) == HAL_OK);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	assert(HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig) == HAL_OK);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	assert (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) == HAL_OK);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 10;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	assert(HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);

	HAL_TIM_MspPostInit(&htim3);
}


static void MX_TIM4_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 20999;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 3;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	assert(HAL_TIM_Base_Init(&htim4) == HAL_OK);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	assert(HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) == HAL_OK);

	assert(HAL_TIM_PWM_Init(&htim4) == HAL_OK);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	assert(HAL_TIM_SlaveConfigSynchronization(&htim4, &sSlaveConfig) == HAL_OK);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	assert(HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) == HAL_OK);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	assert(HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);

	HAL_TIM_MspPostInit(&htim4);
}


static void RTC_Init()
{
	// Вот тут все нифига не по канонам, так как HAL
	// не умеет инициализировать RTC без его перезапуска, что нам совсем не нужно

	__HAL_RTC_RESET_HANDLE_STATE(&hrtc);

	// Вот это реально нужно указать
	hrtc.Instance = RTC;
	// Это указываем скорее для справки
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;


	if (RCC->BDCR & RCC_BDCR_RTCEN)
	{
		// Если в нашем backup домене указано что RTC включен
		// значит с ним мы и будем работать не меняя его настроек

		// Делаем вид что все настроилось как задумано
		hrtc.Lock = HAL_UNLOCKED;
		hrtc.State = HAL_RTC_STATE_READY;

		// Включаем доступ к RTC по соответсвующей APB шине (наверное)
		__HAL_RCC_RTC_ENABLE();

		// И продолжаем работать с RTC
	}
	else
	{


		// Запускаем RTC как положено
		assert(HAL_RTC_Init(&hrtc) == HAL_OK);
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	*/
	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
	Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 0x0;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
	Error_Handler();
	}
	/** Enable the Alarm A
	*/
	sAlarm.AlarmTime.Hours = 0x0;
	sAlarm.AlarmTime.Minutes = 0x0;
	sAlarm.AlarmTime.Seconds = 0x0;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 0x1;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
	{
	Error_Handler();
	}
	/* USER CODE BEGIN R
}


void TIM2_IRQHandler()
{
	HAL_TIM_IRQHandler(&htim2);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (&htim2 == htim)
	{
		_gps_week++;
	}
}


void time_svc_init(void)
{
	// Инициализируем таймеры и все сопутствующие им железочки
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();

	// Включаем PWM выводы для всех таймеров, для того, чтобы пины
	// перешли в OUTPUT режим и ставим счетчик таймера
	// в предельное значение
	// При наших настройках (PWM MODE 1, POLARITY HIGH) это выводит PWM линию в 0
	// Но при этом таймер по пуску сразу же сгенерирует UPD событие и сбросится не делая(? FIXME) лишних тиков
	__HAL_TIM_SET_COUNTER(&htim2, __HAL_TIM_GET_AUTORELOAD(&htim2));
	TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

	__HAL_TIM_SET_COUNTER(&htim3, __HAL_TIM_GET_AUTORELOAD(&htim3));
	TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

	__HAL_TIM_SET_COUNTER(&htim4, __HAL_TIM_GET_AUTORELOAD(&htim4));
	TIM_CCxChannelCmd(htim4.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

	// Подождем чуть чуть, чтобы все PWM линии надежно опустились вниз
	// и все таймеры одновременно стартанули красивым передним фронтом на своих PWM линиях
	HAL_Delay(10);

	// Запускаем все таймеры
	// именно в таком порядке
	__HAL_TIM_ENABLE(&htim2);
	__HAL_TIM_ENABLE(&htim3);
	__HAL_TIM_ENABLE(&htim4);
}
