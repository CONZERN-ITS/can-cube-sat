/*
 * gps_nmea.c
 *
 *  Created on: 16 дек. 2017 г.
 *      Author: developer
 */

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "math.h"

#include "diag/Trace.h"

//#include "FreeRTOS.h"
//#include "task.h"

#include "minmea.h"
#include "gps_nmea.h"
#include "state.h"


static UART_HandleTypeDef uart_GPS;
struct minmea_sentence_gga frame;

float coord_koef = 1853.31;


float tocoord(struct minmea_float *f)
{
    if (f->scale == 0)
        return NAN;
    int_least32_t minutes = f->value % (f->scale * 100);
    return (float) minutes / f->scale;
}


void GPS_Init(void)
{
	uint8_t error = 0;

	if (GPS)
	{
		//	Инициализация USART2 для работы с GPS
		uart_GPS.Instance = USART2;
		uart_GPS.Init.BaudRate = 9600;
		uart_GPS.Init.WordLength = UART_WORDLENGTH_8B;
		uart_GPS.Init.StopBits = UART_STOPBITS_1;
		uart_GPS.Init.Parity = UART_PARITY_NONE;
		uart_GPS.Init.Mode = UART_MODE_TX_RX;
		uart_GPS.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		uart_GPS.Init.OverSampling = UART_OVERSAMPLING_16;

		PROCESS_ERROR(HAL_UART_Init(&uart_GPS));

end:
	state_system.GPS_state = error;
	}
}


void USART2_IRQHandler(void)
{
	uint8_t tmp;
		//Проверка флага о приеме байтика по USART
		if ((USART3->SR & USART_SR_RXNE) != 0)
		{
			//Сохранение принятого байтика
			tmp = USART3->DR;
			if (tmp == (uint8_t)'$')
				need_transfer_data = 1;
		}

	float _lon = tocoord(&frame.longitude);
	float _lat = tocoord(&frame.latitude);
	float _height = minmea_tofloat(&frame.altitude);
	float _time = minmea_tofloat((struct minmea_float *)(&frame.time));

	stateGPS.coordinates[0] = (_lon  - state_zero.zero_GPS[0]) * coord_koef; // FIXME: подумать как лучше сделать получение zeroGPS
	stateGPS.coordinates[1] = (state_zero.zero_GPS[1] - _lat) * coord_koef;
	stateGPS.coordinates[2] = _height;
	stateGPS.time = _time;
}

