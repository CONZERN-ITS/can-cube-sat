/*
 * gps.c
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#include "gps.h"

#include <assert.h>

#include <stm32f4xx_ll_usart.h>
#include <stm32f4xx_ll_dma.h>

#define GPS_UART USART2
#define GPS_DMA DMA1
#define GPS_DMA_STREAM LL_DMA_STREAM_5

#define GPS_DMA_BUFFER_SIZE 1500


static volatile uint8_t _gps_uart_dma_buffer[GPS_DMA_BUFFER_SIZE];
static int _gps_uart_dma_tail;

static uint8_t _gps_sparser_buffer[500];
static ubx_sparser_ctx_t sparser_ctx;


void gps_setup(ubx_sparser_packet_callback_t callback, void * callback_arg)
{
	LL_USART_Disable(GPS_UART);
	LL_DMA_SetPeriphAddress(GPS_DMA, GPS_DMA_STREAM, LL_USART_DMA_GetRegAddr(GPS_UART));
	LL_DMA_SetMemoryAddress(GPS_DMA, GPS_DMA_STREAM, (uint32_t)&_gps_uart_dma_buffer);
	LL_DMA_SetDataLength(GPS_DMA, GPS_DMA_STREAM, sizeof(_gps_uart_dma_buffer));
	_gps_uart_dma_tail = 0;


	LL_DMA_EnableStream(GPS_DMA, GPS_DMA_STREAM);
	LL_USART_EnableDMAReq_RX(GPS_UART);
	LL_USART_EnableIT_RXNE(GPS_UART);
	LL_USART_Enable(GPS_UART);

	ubx_sparser_reset(&sparser_ctx);
	ubx_sparser_set_pbuffer(&sparser_ctx, _gps_sparser_buffer, sizeof(_gps_sparser_buffer));
	ubx_sparser_set_packet_callback(&sparser_ctx, callback, callback_arg);
}


int gps_poll()
{
	DMA_Stream_TypeDef * stream = __LL_DMA_GET_STREAM_INSTANCE(GPS_DMA, GPS_DMA_STREAM);
	assert(stream);

	const int dma_head = (GPS_DMA_BUFFER_SIZE - stream->NDTR);

	int bytes_polled = 0;
	while (_gps_uart_dma_tail != dma_head)
	{
		uint8_t byte = _gps_uart_dma_buffer[_gps_uart_dma_tail];
		ubx_sparser_consume_byte(&sparser_ctx, byte);

		_gps_uart_dma_tail++;
		if (_gps_uart_dma_tail >= GPS_DMA_BUFFER_SIZE)
			_gps_uart_dma_tail = 0;

		bytes_polled++;
	}

	return bytes_polled;
}

