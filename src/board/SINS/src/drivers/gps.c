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


#include "state.h"
#include "gps.h"

/////////////////////////////////////
//#define    DWT_CYCCNT    *(volatile uint32_t* )0xE0001004
//#define    DWT_CONTROL   *(volatile uint32_t* )0xE0001000
//#define    SCB_DEMCR     *(volatile uint32_t* )0xE000EDFC
////FIXME: delete this or not
//char str[16] = {0,};
//uint32_t count_tic = 0;
//
//SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;// разрешаем использовать DWT
//	DWT_CONTROL|= DWT_CTRL_CYCCNTENA_Msk; // включаем счётчик
//
//DWT_CYCCNT = 0;// обнуляем счётчик
//count_tic = DWT_CYCCNT;
//
/////////////////////////////////////


uint8_t *head, *next_head, *tail, *next_tail, *begin, *end, first, second;

uint8_t uart_GPS_buffer[GPS_BUFFER_SIZE] = {0};		// циклический буффер для приема байт по uart
uint8_t packet_buffer[GPS_BUFFER_SIZE] = {0};			// буфер для храниения пакетов


uint64_t epoch_time_ms = 0;
uint64_t next_pps_time_ms = 0;

gps_ctx_t gps;


void uartGPSInit(UART_HandleTypeDef * uart)
{
	uint8_t error = 0;

	uart->Instance = USART2;					//uart для приема GPS
	uart->Init.BaudRate = 9600;
	uart->Init.WordLength = UART_WORDLENGTH_8B;
	uart->Init.StopBits = UART_STOPBITS_1;
	uart->Init.Parity = UART_PARITY_NONE;
	uart->Init.Mode = UART_MODE_TX_RX;
	uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart->Init.OverSampling = UART_OVERSAMPLING_16;

	error = HAL_UART_Init(uart);
	trace_printf("GPS UART init error: %d\n", error);

	//Включение прерывания USART: RXNE
		USART2->CR1 |= USART_CR1_RXNEIE;
		HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);

	gps_init(&gps);

}


//void init_GPS_DMA_logs(){
//	uint8_t error;
//
//	__HAL_RCC_DMA2_CLK_ENABLE();
//	//	Инициализация DMA2_Stream5 для работы c GPS через USART
//	dmaGPS.Instance = DMA2_Stream5;
//	dmaGPS.Init.Channel = DMA_CHANNEL_4;						// 4 канал - на USART2_RX
//	dmaGPS.Init.Direction = DMA_PERIPH_TO_MEMORY;				// направление - из периферии в память
//	dmaGPS.Init.PeriphInc = DMA_PINC_DISABLE;					// инкрементация периферии выключена
//	dmaGPS.Init.MemInc = DMA_MINC_ENABLE;						// инкрементация памяти включена
//	dmaGPS.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;		// длина слова в периферии - байт
//	dmaGPS.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		// длина слова в памяти - байт
//	dmaGPS.Init.Mode = DMA_CIRCULAR;							// режим - обычный
//	dmaGPS.Init.Priority = DMA_PRIORITY_HIGH;					// приоритет - средний
//	dmaGPS.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//	dmaGPS.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
//	dmaGPS.Init.MemBurst = DMA_MBURST_SINGLE;
//	dmaGPS.Init.PeriphBurst = DMA_PBURST_SINGLE;
//	PROCESS_ERROR(HAL_DMA_Init(&dmaGPS));
//
//	// запускаем ДМА на трансфер данных
//	PROCESS_ERROR(HAL_DMA_Start_IT(
//			&dmaGPS, (uint32_t)&uartGPS.Instance->DR,
//			(uint32_t)&dma_GPS_buffer, DMA_GPS_BUFFER_SIZE
//	));
//
//	/*	Enable the DMA transfer for the receiver request by setting the DMAR bit
//	in the UART CR3 register		*/
//	SET_BIT(uartGPS.Instance->CR3, USART_CR3_DMAR);
//
//	end:
//	trace_printf("DMA not init");
//}


void USART2_IRQHandler(void)
{
	uint8_t tmp;
	//Проверка флага о приеме байтика по USART
	if ((USART2->SR & USART_SR_RXNE) != 0)
	{
		//Сохранение принятого байтика
		tmp = USART2->DR;

		next_head = head + 1;
		*head = tmp;
		if (next_head == end) 	next_head = begin;
		if (next_head == tail)	return;
		head = next_head;
	}
}


void initInterruptPin()
{
	__GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpioa;
	gpioa.Mode = GPIO_MODE_IT_RISING;
	gpioa.Pin = GPIO_PIN_0;
	gpioa.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpioa);

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}


void EXTI0_IRQHandler()
{
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


int parse_ubx_nav_sol(uint8_t * payload)		//TODO: дописать
{
	// обрезание пакета до 48 байт т.к. дальше идут зарезервированные поля
	//сбор mavlink пакета
	return 0;
}


int parse_ubx_tim_tp(uint8_t * payload)			// время GPS
{
	uint32_t tow_ms = *(uint32_t * )payload;
	uint16_t week = *(uint16_t * )(payload + 12);		// +12 т.к. время в неделях с 12 по 14 байт

	next_pps_time_ms = (uint64_t)week * 604800000 + (uint64_t)tow_ms;			//	604800000 - кол-во миллисекунд в неделе

	return 0;
}


int parse_ubx_nav_timegps(uint8_t * payload)
{
	uint32_t tow_ms = *(uint32_t * )payload;
	uint16_t week = (uint16_t)(*(int16_t * )(payload + 8));		// +8 т.к. время в неделях с 8 по 10 байт

	epoch_time_ms = (uint64_t)week * 604800000 + (uint64_t)tow_ms;			//	604800000 - кол-во миллисекунд в неделе

	return 0;
}


void gps_init(gps_ctx_t * ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->state = GPS_STATE_SYNC_SEARCH;

	head = &uart_GPS_buffer[0];
	trace_printf("%x", head);
	tail = &uart_GPS_buffer[0];
	begin = &uart_GPS_buffer[0];
	end = begin + GPS_BUFFER_SIZE;
	first = 0;
	second = 0;

	initInterruptPin();
}

int read_gps_buffer()
{

//	for (int i = 0; i < 3; i++)
//	{
		next_tail = tail + 1;
		if (next_tail == end)	next_tail = begin;
		if (next_tail == head)	return -3;
		gps_consume_byte(&gps, *tail);
//		HAL_Delay(100);
		tail = next_tail;
//	}
	int sta = gps.state;
//	trace_printf("%d\n", sta);
	return 0;
}

/*  Функция для разбора пакетов
 *	packet - указатель на начало буфера пакета
 *	packet_size - длина пакета
 */
int process_gps_packet(uint8_t * packet, size_t packet_size)
{
	uint16_t class_id = *(uint16_t *)packet;
	uint8_t * packet_payload = packet + 4;			//указатель на начало полезных данных // +4 - пропуск заголовка

	//check crc
	uint8_t crc_a = 0, crc_b = 0;
	int crc_check_len = packet_size - 2;
	for (int i = 0; i < crc_check_len; i++)		//алгоритм подсчета контрольной суммы
	{
		crc_a += *(packet + i);
		crc_b += crc_a;
	}
	if (*(packet + packet_size - 2) != crc_a && *(packet + packet_size -1) != crc_b)	return -5;		//проверка контрольной суммы

	int parse_error = 0;
	if (class_id == CLASS_ID_UBX_NAV_TIMEGPS)	parse_error = parse_ubx_nav_timegps(packet_payload);
	if (class_id == CLASS_ID_UBX_TIM_TP)	parse_error |= parse_ubx_tim_tp(packet_payload);
	if (class_id == CLASS_ID_UBX_NAV_SOL)	parse_error |= parse_ubx_nav_sol(packet_payload);

	return parse_error;
}


uint8_t header_invalid(uint16_t class_id, uint16_t lenght)
{
	if (class_id == CLASS_ID_UBX_NAV_TIMEGPS && lenght == LEN_UBX_NAV_TIMEGPS)	return 0;
	if (class_id == CLASS_ID_UBX_TIM_TP && lenght == LEN_UBX_TIM_TP)	return 0;
	if (class_id == CLASS_ID_UBX_NAV_SOL && lenght == LEN_UBX_NAV_SOL)	return 0;

	//если дошли сюда, то какая-то лажа с заголовком пакета
	return 1;
}


void gps_consume_byte(gps_ctx_t * ctx, uint8_t byte)
{
	switch (ctx->state)
	{
		case GPS_STATE_SYNC_SEARCH:
			ctx->sw_buffer = (ctx->sw_buffer << 8 | byte);		//записываем синхрослово
			if (ctx->sw_buffer == UBX_SYNCWORD_VALUE)
			{
				ctx->bytes_accum = 0;
				ctx->state = GPS_STATE_HEADER_ACCUM;
			}
		break;

		case GPS_STATE_HEADER_ACCUM:
			if (ctx->bytes_accum < sizeof(ctx->packet_buffer))
			{
				ctx->packet_buffer[ctx->bytes_accum] = byte;
				ctx->bytes_accum++;
			}
			else
			{
//				assert(false);
				int i = 0;
			}

			if (ctx->bytes_accum == 4)
			{
				ctx->expected_packet_size = *(uint16_t * )(&ctx->packet_buffer[2]);		// [2] - звятие длины пакета, +4 - длина заголовка, +2 - длина контрольной суммы
				ctx->state = GPS_STATE_PACKET_ACCUM;
				if (header_invalid(*(uint16_t * )&ctx->packet_buffer[0], *(uint16_t * )&ctx->packet_buffer[2]))
				{
					ctx->state = GPS_STATE_SYNC_SEARCH;
					ctx->sw_buffer = 0;
				}
				else
				{
					ctx->state = GPS_STATE_PACKET_ACCUM;
				}
			}
		break;

		case GPS_STATE_PACKET_ACCUM:
//			trace_printf("im here");
			if (ctx->bytes_accum < sizeof(ctx->packet_buffer))
			{
				ctx->packet_buffer[ctx->bytes_accum] = byte;
				ctx->bytes_accum++;
			}

			if (ctx->bytes_accum == ctx->expected_packet_size)
			{
				process_gps_packet(ctx->packet_buffer, ctx->expected_packet_size);
				ctx->state = GPS_STATE_SYNC_SEARCH;
			}
		break;
	}

}

