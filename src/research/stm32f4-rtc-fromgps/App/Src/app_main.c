#include "main.h"

#include <stdio.h>
#include <assert.h>

#include "ubx_stream_parser.h"


#define GPS_UART USART2
#define GPS_DMA DMA1
#define GPS_DMA_STREAM LL_DMA_STREAM_5

#define GPS_DMA_BUFFER_SIZE 1500


static volatile uint8_t _gps_uart_dma_buffer[GPS_DMA_BUFFER_SIZE];
static int _gps_uart_dma_tail;

static uint8_t _gps_sparser_buffer[500];
static ubx_sparser_ctx_t sparser_ctx;


static void on_ubx_packet(void * userarg, const ubx_any_packet_t * packet)
{
	(void)userarg;

	switch (packet->pid)
	{
	case UBX_PID_NAV_TIMEGPS:
		printf("got gps_time packet: week %d, ms: %lu\r\n",
				packet->packet.gpstime.week,
				packet->packet.gpstime.tow_ms
		);
		break;

	case UBX_PID_TIM_TP:
		printf("got tim_tp packet: week %d, ms: %lu\r\n",
				packet->packet.timtp.week,
				packet->packet.timtp.tow_ms
		);
		break;

	default:
		printf("got packet! pid = 0x%04X\r\n", (int)packet->pid);
	}
}


static void setup_gps_dma()
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
	ubx_sparser_set_packet_callback(&sparser_ctx, on_ubx_packet, NULL);
}


static void poll_gps_dma()
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

	printf("dma polled %d, ubx_state = %d\r\n", bytes_polled, sparser_ctx.state);
}


static void setup_rtc()
{
	//HAL_RTC_Init(&hrtc);
}


int app_main()
{
	// для pritnf
	__HAL_UART_ENABLE(&huart1);

	// для GPS
	setup_gps_dma();

	while(1)
	{
		poll_gps_dma();
		HAL_Delay(1000);
	}

	return 0;
}
