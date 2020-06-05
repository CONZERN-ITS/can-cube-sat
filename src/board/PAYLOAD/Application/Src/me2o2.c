/*
 * me2o2.c
 *
 *  Created on: 31 мая 2020 г.
 *      Author: snork
 */

#include "me2o2.h"

#include <stm32f1xx_hal.h>

#include <its-time.h>

#include "analog.h"

// Нагрузочный резистор сенсора (в Омах)
#define ME2O2_R_LOAD (100)

// Коэффициент усилиния операционного усилителя
#define ME202_OA_AMP (121) // Через делитель в обратной связи в 12 кОм и 100 Ом

// Коэффициент a при формуле пересчета тока из попугаев в проценты концентрации ax + b
#define ME2O2_COEFF_A (0.21091302582667465)

// Коэффициент b при формуле пересчета тока из попугаев в проценты концентрации ax + b
#define ME2O2_COEFF_B (0.0)



int me2o2_init()
{
	// Ацп уже настроено...
	return 0;
}


// Добывает значение через АЦП и пересчитывает его проценты концентрации
static int _read(float * value)
{
	int error = 0;
	// Берем значение с АЦП
	const int oversamapling = 10;
	uint32_t sum_raw = 0;
	uint16_t raw;
	for (int i = 0; i < oversamapling; i++)
	{
		error = its_pld_analog_get_raw(ITS_PLD_ANALOG_TARGET_ME202_O2, &raw);
		if (0 != error)
			return error;

		sum_raw += raw;
	}

	raw = sum_raw / oversamapling;
	float amp_mv = raw * 3300.0f/0xFFF;

	// Окей, у нас есть милливольты с операционного усилителя.
	// ОУ усиливает напряжение на нагрузочном резисторе датчика в 121 раз
	// (через делитель в обратной связи усилителя с плечами в 12 кОм и 100 Ом)
	// Считаем сколько милиливольт реально упало на нагрузочном резисторе
	float res_mv = amp_mv / ME202_OA_AMP;

	// Теперь по закону Ома считаем ток, который идет через нагрузочный резистор
	float sensor_ma = res_mv / ME2O2_R_LOAD ;

	*value =  ME2O2_COEFF_A * (sensor_ma * 1000) + ME2O2_COEFF_B; // * 1000 потому что мА в мкА
	return 0;
}


int me2o2_read(mavlink_pld_me2o2_data_t * msg)
{

	// Берем время
	its_time_t time;
	its_gettimeofday(&time);

	msg->time_s = time.sec;
	msg->time_us = time.usec;

	int error = 0;
	error = _read(&msg->o2_conc);

	return error;
}
