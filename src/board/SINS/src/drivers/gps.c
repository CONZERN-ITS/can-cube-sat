/*
 * gps.c
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#include <stdint.h>
#include <stm32f4xx.h>
#include <diag/Trace.h>


#include "state.h"
#include "gps.h"

uint8_t *head, *next_head, *tail, *next_tail, *begin, *end, first, second;
//			описание UBX пакета			//
uint8_t sync_char_1 = 0xB5, sync_char_2 = 0x62;
uint8_t class, id;
uint16_t lenght;
uint8_t crc_1, crc_2;

uint8_t uart_GPS_buffer[GPS_BUFFER_SIZE] = {0};		// циклический буффер для приема байт по uart
uint8_t packet_buffer[GPS_BUFFER_SIZE] = {0};			// буфер для храниения пакетов


void uartGPSInit(UART_HandleTypeDef * uart)
{
	uint8_t error = 0;

	uart->Instance = USART2;					//uart для приема GPS
	uart->Init.BaudRate = 115200;
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
		if (next_head == end) next_head = begin;
		if (next_head == tail) return;
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


void gps()
{
	head = &uart_GPS_buffer[0];
	tail = &uart_GPS_buffer[0];
	begin = &uart_GPS_buffer[0];
	end = begin + GPS_BUFFER_SIZE;
	first = 0;
	second = 0;
}

uint8_t read_GPS_buffer()
{
	uint8_t parse = 1;
	uint8_t * number_now_element = &packet_buffer[0];
	second = *tail;

	while (parse)
	{
		uint8_t distance_to_end = end - tail;
		if (distance_to_end < 5)						// выполняем проверку на то, что весь пакет находится в буфере
		{
			lenght = *(begin + (4 - (end - tail)));
		}

		while ((first != sync_char_1) && (second != sync_char_2))
		{
			first = second;
			next_tail = tail + 1;
			if (next_tail == end) next_tail = begin;
			if (next_tail == head) return 0;
			second = *next_tail;
			tail = next_tail;
		}
		uint8_t * start = number_now_element;		//указатель на начало пакета
		*number_now_element = first;				//запись синхрослов
		*(number_now_element + 1) = second;
		number_now_element += 2;




	}

}


