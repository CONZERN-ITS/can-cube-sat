/*
 * uplink.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#include "uplink.h"

#include <stm32f4xx_hal.h>
#include <diag/Trace.h>

#include "common.h"

#include "watchdog.h"


static UART_HandleTypeDef huplink_uart;
static int uart_error_count = 0;


int uplink_init(void)
{
	HAL_StatusTypeDef hal_error;

	huplink_uart.Instance = USART1;					//uart для отправки данных на ESP
	huplink_uart.Init.BaudRate = 57600;
	huplink_uart.Init.WordLength = UART_WORDLENGTH_8B;
	huplink_uart.Init.StopBits = UART_STOPBITS_1;
	huplink_uart.Init.Parity = UART_PARITY_NONE;
	huplink_uart.Init.Mode = UART_MODE_TX_RX;
	huplink_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huplink_uart.Init.OverSampling = UART_OVERSAMPLING_16;

	hal_error = HAL_UART_Init(&huplink_uart);
	if (hal_error != HAL_OK)
	{
		int error = sins_hal_status_to_errno(hal_error);
		trace_printf("Transfer UART init error: %d\n", error);
		return error;
	}

	return 0;
}


int debug_uart_init(void)
{
	HAL_StatusTypeDef hal_error;

	huplink_uart.Instance = USART3;					//uart для отправки данных на ESP
	huplink_uart.Init.BaudRate = 115200;
	huplink_uart.Init.WordLength = UART_WORDLENGTH_8B;
	huplink_uart.Init.StopBits = UART_STOPBITS_1;
	huplink_uart.Init.Parity = UART_PARITY_NONE;
	huplink_uart.Init.Mode = UART_MODE_TX_RX;
	huplink_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huplink_uart.Init.OverSampling = UART_OVERSAMPLING_16;

	hal_error = HAL_UART_Init(&huplink_uart);
	if (hal_error != HAL_OK)
	{
		int error = sins_hal_status_to_errno(hal_error);
//		trace_printf("Transfer UART init error: %d\n", error);
		return error;
	}

	return 0;
}


int uplink_write_raw(const void * data, int data_size)
{
	HAL_StatusTypeDef hal_error;

	hal_error = HAL_UART_Transmit(&huplink_uart, (uint8_t*)data, data_size, HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
	{
		uart_error_count++;
		if (uart_error_count >= 10)
			HAL_NVIC_SystemReset();
		__HAL_RCC_USART1_FORCE_RESET();
		HAL_Delay(1);
		__HAL_RCC_USART1_RELEASE_RESET();
		hal_error = HAL_UART_Init(&huplink_uart);
		return sins_hal_status_to_errno(hal_error);
	}


	return 0;
}


int uplink_write_mav(const mavlink_message_t * msg)
{
	// https://mavlink.io/en/about/overview.html#mavlink-2-packet-format
	static uint8_t msg_buffer[280]; // 280 максимальный размер MAV пакета версии 2

	uint16_t len = mavlink_msg_to_send_buffer(msg_buffer, msg);
	int error = uplink_write_raw(msg_buffer, len);
	if (error)
		return error;

	iwdg_reload(&transfer_uart_iwdg_handle);
	return 0;
}
