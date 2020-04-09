/*
 * gps.c
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#include "gps.h"

#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stm32f4xx.h>
#include <diag/Trace.h>

#include "sins_config.h"
#include "time_svc/time_util.h"
#include "time_svc/time_svc.h"


//! Хендл для уарта для GPS
static UART_HandleTypeDef uartGPS;

//! Циклобуфер для входящих байт GPS
static uint8_t _uart_cycle_buffer[ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE];
static int _uart_cycle_buffer_head;
static int _uart_cycle_buffer_tail;

//! Линейный буфер для сбора сообщений потоковым парсером UBX
static uint8_t _ubx_sparser_buffer[ITS_SINS_GPS_UBX_SPARSER_BUFFER_SIZE];
//! Контекст потокового парсера UBX
static ubx_sparser_ctx_t _sparser_ctx;


//! Колбек пользователя. Мы устанавливаем свой, но пользователю тоже нужно данные
static ubx_sparser_packet_callback_t _user_packet_callback;

//! Время на следующий фронт PPS
/*! Нулем оно быть не может, поэтому ноль показывает отсутствие
 *  этих данных как таковых */
static time_t _next_pps_time = 0;

//! Колбек для входящих GPS пакетов.
/*! Некоторые мы будем перехватывать для уточнения службы времени */
void _internal_packet_callback(void * user_arg, const ubx_any_packet_t * packet_)
{
	// FIXME: Возможно стоит что-то сделать, чтобы не использовать оба пакета на одном такте?
	// Если они оба придут (а они оба придут)
	// Возможно стоит выключить TIMTP в целом?
	switch (packet_->pid)
	{
	case UBX_PID_NAV_TIMEGPS:
		{
			struct timeval tmv;
			const ubx_gpstime_packet_t * packet = &packet_->packet.gpstime;
			// Мы ждем следующей секунды, поэтому округляем до следующей секунды
			uint32_t next_tow_ms = packet->tow_ms;
			next_tow_ms = ((next_tow_ms + 999) / 1000) * 1000;
			gps_time_to_unix_time(packet->week, next_tow_ms, &tmv);
			_next_pps_time = tmv.tv_sec;
		}
		break;

	case UBX_PID_TIM_TP:
		{
			struct timeval tmv;
			// Это сообщение ровно о том, что нам надо и говорит
			const ubx_timtp_packet_t * packet = &packet_->packet.timtp;
			gps_time_to_unix_time(packet->week, packet->tow_ms, &tmv);
			_next_pps_time = tmv.tv_sec;
		}
		break;

	default:
		break;
	}

	// В конце передаем таки пакет пользователю
	_user_packet_callback(user_arg, packet_);
}


//! Настойка уартовой перефирии
static void _init_uart()
{
	uartGPS.Instance = USART2;					//uart для приема GPS
	uartGPS.Init.BaudRate = 9600;
	uartGPS.Init.WordLength = UART_WORDLENGTH_8B;
	uartGPS.Init.StopBits = UART_STOPBITS_1;
	uartGPS.Init.Parity = UART_PARITY_NONE;
	uartGPS.Init.Mode = UART_MODE_TX_RX;
	uartGPS.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uartGPS.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_UART_Init(&uartGPS);

	//Включение прерывания USART: RXNE
	__HAL_UART_ENABLE_IT(&uartGPS, UART_IT_RXNE);

	// nvic в hal_msp
}


void NONHAL_PPS_MspInit(void);


//! настройка перфирии для PPS сигнала
static void _init_pps()
{
	// Настройка PPS пина на прерывания
	__GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpioa;
	gpioa.Mode = GPIO_MODE_IT_RISING;
	gpioa.Pin = GPIO_PIN_0;
	gpioa.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpioa);

	// nvic в hal_msp
	NONHAL_PPS_MspInit();
}


void USART2_IRQHandler(void)
{
	//Проверка флага о приеме байтика по USART
	if ((USART2->SR & USART_SR_RXNE) != 0)
	{
		//Сохранение принятого байтика
		volatile uint8_t tmp = USART2->DR;

		int next_head = _uart_cycle_buffer_head + 1;
		if (next_head >= ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE)
			next_head = 0;

		if (next_head == _uart_cycle_buffer_tail)
			return; // Переполнение циклобуфера. Выкидываем байт

		_uart_cycle_buffer[_uart_cycle_buffer_head] = tmp;
		__disable_irq();
		_uart_cycle_buffer_head = next_head;
		__enable_irq();
	}
}


void EXTI0_IRQHandler()
{
	// Правим службу времени
	if (_next_pps_time > 0)
		time_svc_world_set_time(_next_pps_time);

	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
}


void gps_init(
		gps_packet_callback_t packet_callback, void * packet_callback_arg
)
{
	// У нас пока ничего не приходило и времени на следующую метку у нас нет
	time_t next_time = 0;

	// Настриваем потоковый парсер UBX пакетов
	_user_packet_callback = packet_callback;
	ubx_sparser_reset(&_sparser_ctx);
	ubx_sparser_set_pbuffer(&_sparser_ctx, _ubx_sparser_buffer, sizeof(_ubx_sparser_buffer));
	ubx_sparser_set_packet_callback(&_sparser_ctx, _internal_packet_callback, packet_callback_arg);

	// Настраиваем циклобуфер для уарта
	_uart_cycle_buffer_head = 0;
	_uart_cycle_buffer_tail = 0;

	// Настраиваем уартовое железо
	_init_uart();

	// Настраиваем железо для работы с PPS
	_init_pps();

	// Все готово! поехали!
}


int gps_poll(void)
{
	size_t i = 0;
	// Выгребаем байты из цилобуфера и корими ими стрим парсер
	for ( ; i < ITS_SINS_GPS_MAX_POLL_SIZE; i++)
	{
		if (_uart_cycle_buffer_tail == _uart_cycle_buffer_head)
		{
			// Буфер опустел, мы тут закончили
			break;
		}

		// Достаем байт
		uint8_t byte = _uart_cycle_buffer[_uart_cycle_buffer_tail];
		int next_tail = _uart_cycle_buffer_tail + 1;
		if (next_tail >= ITS_SINS_GPS_UART_CYCLE_BUFFER_SIZE)
			next_tail = 0;

		// Показываем что мы его достали
		__disable_irq();
		_uart_cycle_buffer_tail = next_tail;
		__enable_irq();

		// кормим стримпарсер этим байтом
		// если какой-то пакет успешно распарсился, то парсер вызовет колбеки
		ubx_sparser_consume_byte(&_sparser_ctx, byte);
	}

	return i;
}

