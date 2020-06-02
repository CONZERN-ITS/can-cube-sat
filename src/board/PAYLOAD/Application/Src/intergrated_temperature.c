
#include "integrated_temperature.h"

#include <its-time.h>

#include "analog.h"


// Напряжение со встроенного термистра при 25 градусах (в милливольта)
#define INTERNAL_TEMP_V25 (1430.0f)
// Коэффициент k внутренного термистра (мВ/C
#define INTERNAL_TEMP_AVG_SLOPE (4.3f)



int its_pld_inttemp_init(void)
{

	return 0;
}


int its_pld_inttemp_read(mavlink_own_temp_t * msg)
{
	int error = 0;

	its_time_t time;
	its_gettimeofday(&time);

	const int oversampling = 10;
	uint32_t raw_sum = 0;
	uint16_t raw;
	for (int i = 0; i < oversampling; i++)
	{
		error = its_pld_analog_get_raw(ITS_PLD_ANALOG_TARGET_INTEGRATED_TEMP, &raw);
		if (0 != error)
			return error;

		raw_sum += raw;
	}

	raw = raw_sum / oversampling;

	float mv = raw * 3300.0f / 0x0FFF;
	float temp = (INTERNAL_TEMP_V25 - mv) / INTERNAL_TEMP_AVG_SLOPE + 25;

	msg->time_s = time.sec;
	msg->time_us = time.usec;
	msg->deg = temp;

	return 0;
}
