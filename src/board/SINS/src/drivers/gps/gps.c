/*
 * gps.c
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#include <stdint.h>
#include <string.h>
#include <stm32f4xx.h>
#include <diag/Trace.h>

#include "gps.h"
#include "sins_config.h"

//! Хендл для уарта для GPS
static UART_HandleTypeDef uartGPS;

//! Циклобуфер для входящих байт GPS
static uint8_t _uart_cycle_buffer[GPS_UART_CYCLE_BUFFER_SIZE];
static int _uart_cycle_buffer_head;
static int _uart_cycle_buffer_tail;

//! Линейный буфер для сбора сообщений потоковым парсером UBX
static uint8_t _ubx_sparser_buffer[GPS_UBX_SPARSER_BUFFER_SIZE];
//! Контекст потокового парсера UBX
static ubx_sparser_ctx_t _sparser_ctx;


//! Колбек для PPS метки
static gps_pps_callback_t _pps_callback;
//! Пользовательский аргумент для pps метки
static void * _pps_callback_user_arg;


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
	HAL_NVIC_SetPriority(USART2_IRQn, GPS_UART_IRQ_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}


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

	HAL_NVIC_SetPriority(EXTI0_IRQn, GPS_PPS_IRQ_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


void USART2_IRQHandler(void)
{
	//Проверка флага о приеме байтика по USART
	if ((USART2->SR & USART_SR_RXNE) != 0)
	{
		//Сохранение принятого байтика
		volatile uint8_t tmp = USART2->DR;

		int next_head = _uart_cycle_buffer_head + 1;
		if (next_head >= GPS_UART_CYCLE_BUFFER_SIZE)
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
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
}


void gps_init(
		gps_packet_callback_t packet_callback, void * packet_callback_arg,
		gps_pps_callback_t pps_callback, void * pps_callback_arg
)
{
	// Настриваем потоковый парсер UBX пакетов
	ubx_sparser_reset(&_sparser_ctx);
	ubx_sparser_set_pbuffer(&_sparser_ctx, _ubx_sparser_buffer, sizeof(_ubx_sparser_buffer));
	ubx_sparser_set_packet_callback(&_sparser_ctx, packet_callback, packet_callback_arg);

	// Настраиваем циклобуфер для уарта
	_uart_cycle_buffer_head = 0;
	_uart_cycle_buffer_tail = 0;

	// Настраиваем уартовое железо
	_init_uart();

	// Настраиваем колбек для PPS
	_pps_callback = pps_callback;
	_pps_callback_user_arg = pps_callback_arg;

	// Настраиваем железо для работы с PPS
	_init_pps();

	// Все готово! поехали!
}


int gps_poll(void)
{
	size_t i = 0;
	// Выгребаем байты из цилобуфера и корими ими стрим парсер
	for ( ; i < GPS_MAX_POLL_SIZE; i++)
	{
		if (_uart_cycle_buffer_tail == _uart_cycle_buffer_head)
		{
			// Буфер опустел, мы тут закончили
			break;
		}

		// Достаем байт
		uint8_t byte = _uart_cycle_buffer[_uart_cycle_buffer_tail];
		int next_tail = _uart_cycle_buffer_tail + 1;
		if (next_tail >= GPS_UART_CYCLE_BUFFER_SIZE)
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

