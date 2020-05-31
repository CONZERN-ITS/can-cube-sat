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
typedef enum its_pld_analog_target_t
{
	//! NO2 сенсор из MIC-6814
	ITS_PLD_ANALOG_TARGET_MICS6814_NO2,
	//! NH3 сенсор из MIC-6814
	ITS_PLD_ANALOG_TARGET_MICS6814_NH3,
	//! CO сенсор из MIC-6814
	ITS_PLD_ANALOG_TARGET_MICS6814_CO,

	//! O2 сенсор ME202
	ITS_PLD_ANALOG_TARGET_ME202_O2,

	//! Интегрированный в stm32 термистр
	ITS_PLD_ANALOG_TARGET_INTEGRATED_TEMP
} its_pld_analog_target_t;


//! Резистивное плечо для измерения сопротивления сенсоров ME202
/*!
 Схема измерения для каждого сенсора такая:
 vcc 3.3 +--====--+--====--+--====--+--====-- gnd
         |   r1   |   r2   |   r3   |   rx
         |       /        /         |
         |        |        |        |
         +--------+--------+        +---- к АЦП
  r1, r2 и r3 это известные нам резисторы.
  rx - незивестное сопротивление, которое выдает сенсор.
  Поскольку rx может меняться очень сильно, а мы хотим знать его довольно точно,
  мы сделали переменное плечо делителя, с которого снимаем напряжение посредством АЦП.
  Это плечо может состять только из r3, или из r3 + r2 или из r3 + r2 + r1.
  Этот енум обозначает сколько резисторов использутся (или нужно использовать) в плече */
typedef enum its_pld_analog_res_arm {
	//! Используются три резистора в плече
	ITS_PLD_ANALOG_RES_ARM_TRIPLE,
	//! Используются два резистора в плече
	ITS_PLD_ANALOG_RES_ARM_DOUBLE,
	//! Используется один резистор в плече
	ITS_PLD_ANALOG_RES_ARM_SINGLE,
} its_pld_analog_res_arm;

//! Инициализация аналоговой подсистемы
int its_pld_analog_init(void);

//! Чтение сырого значения АЦП для указанного датчика
int its_pld_analog_get_raw(its_pld_analog_target_t target, uint16_t * value);

//! Чтение напряжение с АЦП в милливольтах
/*! Довольно тупая функция, предполагающая что мы питаемся ровно от 3.3 вольта */
int its_pld_analog_get_mv(its_pld_analog_target_t target, float * value);

//! Установка размера плеча резисторного делителя
/*! Работает только для таргетов из MICS6814 */
int its_pld_analog_set_res_arm(its_pld_analog_target_t target, its_pld_analog_res_arm arm);

//! Чтение напряжение резистора rx из делительной схемы
/*! Работает только для таргетов из MICS6814 */
int its_pld_analog_get_res(its_pld_analog_target_t target, float * value);


#endif /* INC_ADC_H_ */
