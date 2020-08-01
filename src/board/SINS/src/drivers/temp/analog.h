/*
 * adc.h
 *
 *  Created on: May 30, 2020
 *      Author: snork
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include <stdint.h>

//! Целевой аналоговый сенсор, с которым требуется работать
typedef enum analog_target_t
{
	//! Интегрированный в stm32 термистр
	ANALOG_TARGET_INTEGRATED_TEMP
} analog_target_t;


//! Инициализация аналоговой подсистемы
int analog_init(void);

//! Реинициализация аналоговой подсистемы
/*! Резет всего до чего дотягиваемся и повторная инициализация */
int analog_restart(void);

//! Чтение сырого значения АЦП для указанного датчика
int analog_get_raw(analog_target_t target, uint16_t * value);


#endif /* INC_ADC_H_ */
