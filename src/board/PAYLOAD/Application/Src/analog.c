/*
 * adc.c
 *
 *  Created on: May 30, 2020
 *      Author: snork
 */

#include "analog.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <stm32f1xx_hal.h>

#include "util.h"


extern ADC_HandleTypeDef hadc1;


#define _ITS_PLD_ADC_HAL_TIMEOUT (HAL_MAX_DELAY)

#define _ITS_PLD_ADC_HANDLE (&hadc1)


// Пины для управления резисторным плечем резистивных сенсоров
#define _ITS_PLD_RES_ARM_CTRL_NO2_R1_VALUE	(10*1000)

#define _ITS_PLD_RES_ARM_CTRL_NO2_2_PORT	GPIOA
#define _ITS_PLD_RES_ARM_CTRL_NO2_2_PIN		GPIO_PIN_4

#define _ITS_PLD_RES_ARM_CTRL_NO2_R2_VALUE	(10*1000)

#define _ITS_PLD_RES_ARM_CTRL_NO2_1_PORT	GPIOA
#define _ITS_PLD_RES_ARM_CTRL_NO2_1_PIN		GPIO_PIN_5

#define _ITS_PLD_RES_ARM_CTRL_NO2_R3_VALUE	(10*1000)


#define _ITS_PLD_RES_ARM_CTRL_NH3_R1_VALUE	(10*1000)

#define _ITS_PLD_RES_ARM_CTRL_NH3_2_PORT	GPIOA
#define _ITS_PLD_RES_ARM_CTRL_NH3_2_PIN		GPIO_PIN_6

#define _ITS_PLD_RES_ARM_CTRL_NH3_R2_VALUE	(10*1000)

#define _ITS_PLD_RES_ARM_CTRL_NH3_1_PORT	GPIOA
#define _ITS_PLD_RES_ARM_CTRL_NH3_1_PIN		GPIO_PIN_7

#define _ITS_PLD_RES_ARM_CTRL_NH3_R3_VALUE	(10*1000)


#define _ITS_PLD_RES_ARM_CTRL_CO_R1_VALUE	(10*1000)

#define _ITS_PLD_RES_ARM_CTRL_CO_2_PORT		GPIOB
#define _ITS_PLD_RES_ARM_CTRL_CO_2_PIN		GPIO_PIN_2

#define _ITS_PLD_RES_ARM_CTRL_CO_R2_VALUE	(10*1000)

#define _ITS_PLD_RES_ARM_CTRL_CO_1_PORT		GPIOB
#define _ITS_PLD_RES_ARM_CTRL_CO_1_PIN		GPIO_PIN_1

#define _ITS_PLD_RES_ARM_CTRL_CO_R3_VALUE	(10*1000)


//! Перевод пина в input и highz состояние
static void _pin_to_input_mode(GPIO_TypeDef * port, uint32_t pin)
{
	GPIO_InitTypeDef init;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_INPUT;
	init.Pin = pin;

	HAL_GPIO_Init(port, &init);
}


//! Перевод пина в output состояние с высоким логическим уровнем
static void _pin_to_high_output(GPIO_TypeDef * port, uint32_t pin)
{
	GPIO_InitTypeDef init;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pin = pin;

	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	HAL_GPIO_Init(port, &init);
}


//! Возвращает true, если пин в состоянии input floating
static bool _pin_is_input(GPIO_TypeDef * port, uint32_t pin)
{
	// Проходим по младшим битам
	uint8_t lower_pin_mask = (uint8_t)(pin & 0xFF);
	for (int i = 0; i < 8; i++)
	{
		if (0 == ((lower_pin_mask >> i) & 0x01))
			continue; // Этот бит нас не интересует

		// Режим порта задают два вот этих бита
		const uint8_t pin_mode_value = (port->CRL >> (4*i + 2)) & 0x03;
		// Если эти два бита имеют значение 0x01, значит пин в состоянии input floating
		if (0x01 != pin_mode_value)
			return false;
	}


	uint8_t upper_pin_mask = (uint8_t)((pin >> 8) & 0xFF);
	for (int i = 0; i < 8; i++)
	{
		if (0 == ((upper_pin_mask >> i) & 0x01))
			continue; // Этот бит нас не интересует

		// Режим порта задают два вот этих бита
		const uint8_t pin_mode_value = (port->CRH >> (4*i + 2)) & 0x03;
		// Если эти два бита имеют значение 0x01, значит пин в состоянии input floating
		if (0x01 != pin_mode_value)
			return false;
	}

	// Если все тесты прошли, то все ок
	return true;
}


//! Создает структуру конфигурации канала АЦП
static int _channgel_config_for_target(its_pld_analog_target_t target, ADC_ChannelConfTypeDef * config)
{
	// Все согласно разводке на плате
	int error = 0;
	switch(target)
	{
	case ITS_PLD_ANALOG_TARGET_MICS6814_NO2:
		config->Channel = ADC_CHANNEL_0;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_NH3:
		config->Channel = ADC_CHANNEL_1;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_CO:
		config->Channel = ADC_CHANNEL_2;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	case ITS_PLD_ANALOG_TARGET_ME202_O2:
		config->Channel = ADC_CHANNEL_3;
		config->Rank = ADC_REGULAR_RANK_1;
		config->SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
		break;

	default:
		error = -ENOSYS;
		break;
	}

	return error;
}


//! Возвращает _координаты_ управляющих пинов для управления плечами резисторного делителя
static int _res_arm_ctrls_for_target(its_pld_analog_target_t target,
		GPIO_TypeDef ** port1, uint32_t * pin1,
		GPIO_TypeDef ** port2, uint32_t * pin2
)
{
	int error = 0;

	switch (target)
	{
	case ITS_PLD_ANALOG_TARGET_MICS6814_NO2:
		*port1 = _ITS_PLD_RES_ARM_CTRL_NO2_1_PORT;
		*pin1 = _ITS_PLD_RES_ARM_CTRL_NO2_1_PIN;
		*port2 = _ITS_PLD_RES_ARM_CTRL_NO2_2_PORT;
		*pin2 = _ITS_PLD_RES_ARM_CTRL_NO2_2_PIN;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_NH3:
		*port1 = _ITS_PLD_RES_ARM_CTRL_NH3_1_PORT;
		*pin1 = _ITS_PLD_RES_ARM_CTRL_NH3_1_PIN;
		*port2 = _ITS_PLD_RES_ARM_CTRL_NH3_2_PORT;
		*pin2 = _ITS_PLD_RES_ARM_CTRL_NH3_2_PIN;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_CO:
		*port1 = _ITS_PLD_RES_ARM_CTRL_CO_1_PORT;
		*pin1 = _ITS_PLD_RES_ARM_CTRL_CO_1_PIN;
		*port2 = _ITS_PLD_RES_ARM_CTRL_CO_2_PORT;
		*pin2 = _ITS_PLD_RES_ARM_CTRL_CO_2_PIN;
		break;

	default:
		error = -ENOSYS;
		break;
	};

	return error;
}


static int _res_arm_value_for_target(its_pld_analog_target_t target, float * value)
{
	GPIO_TypeDef * port1, * port2;
	uint32_t pin1, pin2;

	int error = _res_arm_ctrls_for_target(target, &port1, &pin1, &port2, &pin2);
	if (0 != error)
		return error;

	// нужно выяснить в каком состоянии сейчас плечо
	// это делается в высшей степени неудобно :(
	float r1, r2, r3;
	switch (target)
	{
	case ITS_PLD_ANALOG_TARGET_MICS6814_NO2:
		r1 = _ITS_PLD_RES_ARM_CTRL_NO2_R1_VALUE;
		r2 = _ITS_PLD_RES_ARM_CTRL_NO2_R2_VALUE;
		r3 = _ITS_PLD_RES_ARM_CTRL_NO2_R3_VALUE;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_NH3:
		r1 = _ITS_PLD_RES_ARM_CTRL_NH3_R1_VALUE;
		r2 = _ITS_PLD_RES_ARM_CTRL_NH3_R2_VALUE;
		r3 = _ITS_PLD_RES_ARM_CTRL_NH3_R3_VALUE;
		break;

	case ITS_PLD_ANALOG_TARGET_MICS6814_CO:
		r1 = _ITS_PLD_RES_ARM_CTRL_CO_R1_VALUE;
		r2 = _ITS_PLD_RES_ARM_CTRL_CO_R2_VALUE;
		r3 = _ITS_PLD_RES_ARM_CTRL_CO_R3_VALUE;
		break;

	default:
		return -EINVAL;
	}


	// r3 всегда считается
	float retval = r3;

	// Если управляющий пин 1 отключен, то r2 тоже считается
	if (_pin_is_input(port1, pin1))
	{
		retval += r2;

		// А если еще и управляющий пин 2 отключен, то и r1 тоже считается
		if (_pin_is_input(port2, pin2))
		retval += r1;
	}

	*value = retval;
	return 0;
}


int its_pld_analog_init()
{
	// Предположим, что куб все правильно настроил
	// Нам не нужно никакой автоматики, настриваемся на один канал
	// Вроде вот такого
	//	hadc1.Instance = ADC1;
	//	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	//	hadc1.Init.ContinuousConvMode = DISABLE;
	//	hadc1.Init.DiscontinuousConvMode = DISABLE;
	//	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	//	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	//	hadc1.Init.NbrOfConversion = 1;
	//	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	//	{
	//	Error_Handler();
	//	}

	// Еще нужно настроить GPIO для упрваления плечами
	// но они правильно настроены с резета (в in)
	// поэтому мы их не трогаем тоже
	// Только включим клоки (хотя они и так должны быть включены)
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	return 0;
}


int its_pld_analog_get_raw(its_pld_analog_target_t target, uint16_t * value)
{
	int error = 0;

	ADC_ChannelConfTypeDef config;
	error = _channgel_config_for_target(target, &config);
	if (0 != error)
		return error;

	HAL_StatusTypeDef hal_error = HAL_ADC_ConfigChannel(_ITS_PLD_ADC_HANDLE, &config);
	error = its_pld_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	hal_error = HAL_ADC_Start(&hadc1);
	error = its_pld_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	hal_error = HAL_ADC_PollForConversion(_ITS_PLD_ADC_HANDLE, _ITS_PLD_ADC_HAL_TIMEOUT);
	error = its_pld_hal_status_to_errno(hal_error);
	if (0 != error)
		return error;

	*value = (uint16_t)HAL_ADC_GetValue(_ITS_PLD_ADC_HANDLE);
	return 0;
}


int its_pld_analog_get_mv(its_pld_analog_target_t target, float * value)
{
	int error;
	uint16_t raw;
	error = its_pld_analog_get_raw(target, &raw);
	if (0 != error)
		return error;

	*value = raw * 3.3f/0xFFF;
	return 0;
}


int its_pld_analog_set_res_arm(its_pld_analog_target_t target, its_pld_analog_res_arm arm)
{
	int error;
	GPIO_TypeDef * port1, * port2;
	uint32_t pin1, pin2;

	error = _res_arm_ctrls_for_target(target, &port1, &pin1, &port2, &pin2);
	if (0 != error)
		return error;

	switch (arm)
	{
	case ITS_PLD_ANALOG_RES_ARM_TRIPLE:
		_pin_to_input_mode(port1, pin1);
		_pin_to_input_mode(port2, pin2);
		break;
	case ITS_PLD_ANALOG_RES_ARM_DOUBLE:
		_pin_to_input_mode(port1, pin1);
		_pin_to_high_output(port2, pin2);
		break;
	case ITS_PLD_ANALOG_RES_ARM_SINGLE:
		_pin_to_high_output(port1, pin1);
		_pin_to_high_output(port2, pin2);
		break;
	default:
		error = -EINVAL;
	}

	return error;
}


int its_pld_analog_get_res(its_pld_analog_target_t target, float * value)
{
	int error;

	// Добываем сырое значение с ацп
	uint16_t raw;
	error = its_pld_analog_get_raw(target, &raw);
	if (0 != error)
		return error;// 0x0FFF - максимальное значение ацп == напряжение питания делителя

	// Находим текущее сопротивление плеча для этого таргета
	float arm_value;
	error = _res_arm_value_for_target(target, &arm_value);
	if (0 != error)
		return error;

	// Ну и теперь зная сопротивление плеча пересчитываем rx
	float rx = arm_value * (float)raw/(0x0FFF - raw);
	// выше 0x0FFF - максимальное значение ацп == напряжение питания делителя

	return rx;
}

