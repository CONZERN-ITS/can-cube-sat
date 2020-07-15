/*
 * gps.c
 *
 *  Created on: 14 мар. 2020 г.
 *      Author: developer
 */

#include "gps.h"

#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stm32f4xx.h>
//#include <diag/Trace.h>

#include "config.h"
#include "common.h"
#include "time_util.h"
//#include "time_svc.h"

// ==========================================
// Это для фонового приёма пакетиков
// ==========================================

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

// ==========================================
// Это для работы с PPS
// ==========================================

//! Время на следующий фронт PPS
/*! Нулем оно быть не может, поэтому ноль показывает отсутствие
 *  этих данных как таковых */
static time_t _next_pps_time = 0;

// ==========================================
// Это для конфигурации
// ==========================================

//! Статус ожидания ответа на конфигурационный пакет
static enum {
	GPS_CFG_STATUS_WAIT_ACK,
	GPS_CFG_STATUS_GOT_ACK,
	GPS_CFG_STATUS_GOT_NACK
} _cfg_status;


//! Идентификатор отправленного пакета конфигурацииs
static ubx_pid_t _cfg_sent_pid;


//! Колбек для входящих GPS пакетов.
/*! Некоторые мы будем перехватывать для уточнения службы времени */
void _internal_packet_callback(void * user_arg, const ubx_any_packet_t * packet_)
{
	// FIXME: Возможно стоит что-то сделать, чтобы не использовать оба пакета на одном такте?
	// Если они оба придут (а они оба придут)
	// Возможно стоит выключить TIMTP в целом?
	switch (packet_->pid)
	{
	// Это сообщение используется для службы времени
	case UBX_PID_NAV_TIMEGPS:
		{
			struct timeval tmv;
			const ubx_gpstime_packet_t * packet = &packet_->packet.gpstime;
			// Мы ждем следующей секунды, поэтому округляем до следующей секунды

			if (
					(packet->valid_flags & UBX_NAVGPSTIME_FLAGS__LEAPS_VALID)
					&& (packet->valid_flags & UBX_NAVGPSTIME_FLAGS__TOW_VALID)
					&& (packet->valid_flags & UBX_NAVGPSTIME_FLAGS__WEEK_VALID)
			){
				// Работаем с этим сообщением только если оно валидно
				uint32_t next_tow_ms = packet->tow_ms;
				next_tow_ms = ((next_tow_ms + 999) / 1000) * 1000;
				gps_time_to_unix_time(packet->week, next_tow_ms, &tmv);
				_next_pps_time = tmv.tv_sec - packet->leaps; // вычитаем липосекунды
			}
		}
		break;

	// Это сообщение используется для службы времени
	case UBX_PID_TIM_TP:
		{
			struct timeval tmv;
			// Это сообщение ровно о том, что нам надо и говорит
			const ubx_timtp_packet_t * packet = &packet_->packet.timtp;
			gps_time_to_unix_time(packet->week, packet->tow_ms, &tmv);
			_next_pps_time = tmv.tv_sec;
		}
		break;

	// Это сообщение используется для конфигурации
	case UBX_PID_CFG_ACK:
		{
			// дождалис
			const ubx_ack_packet_t * packet = &packet_->packet.ack;
			if (_cfg_sent_pid == packet->packet_pid)
				_cfg_status = GPS_CFG_STATUS_GOT_ACK;
		}
		break;

	// Это сообщение используется для конфигурации
	case UBX_PID_CFG_NACK:
		{
			// не дождались
			const ubx_nack_packet_t * packet = &packet_->packet.nack;
			if (_cfg_sent_pid == packet->packet_pid)
				_cfg_status = GPS_CFG_STATUS_GOT_NACK;
		}
		break; // на NACK забиваем

	// Прочие сообщения не используем
	default:
		break;
	}

	// В конце передаем таки пакет пользователю
	_user_packet_callback(user_arg, packet_);
}


//! Настойка уартовой перефирии
static int _init_uart()
{
	uartGPS.Instance = USART2;					//uart для приема GPS
	uartGPS.Init.BaudRate = 9600;
	uartGPS.Init.WordLength = UART_WORDLENGTH_8B;
	uartGPS.Init.StopBits = UART_STOPBITS_1;
	uartGPS.Init.Parity = UART_PARITY_NONE;
	uartGPS.Init.Mode = UART_MODE_TX_RX;
	uartGPS.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uartGPS.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_StatusTypeDef hal_error;
	hal_error = HAL_UART_Init(&uartGPS);
	if (HAL_OK != hal_error)
		return sins_hal_status_to_errno(hal_error);

	//Включение прерывания USART: RXNE
	__HAL_UART_ENABLE_IT(&uartGPS, UART_IT_RXNE);

	// nvic в hal_msp
	return 0;
}


//void NONHAL_PPS_MspInit(void);


//! настройка перфирии для PPS сигнала
static void _init_pps()
{
	// Настройка PPS пина на прерывания
	__GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef gpioa;
	gpioa.Mode = GPIO_MODE_IT_RISING;
	gpioa.Pin = GPIO_PIN_0;
	gpioa.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &gpioa);

	// nvic в hal_msp
//	NONHAL_PPS_MspInit();
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

/*
void EXTI0_IRQHandler()
{
	// Правим службу времени
	if (_next_pps_time > 0)
		time_svc_world_set_time(_next_pps_time);


	HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
//	if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_9))
//		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, RESET);
//	else
//		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, SET);

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
*/

int gps_init(
		gps_packet_callback_t packet_callback, void * packet_callback_arg
)
{
	// У нас пока ничего не приходило и времени на следующую метку у нас нет
	_next_pps_time = 0;

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
	return 0;
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



// Конфигурация приёмника
extern const uint8_t * ublox_neo6_cfg_msgs[];


//! Отправка одного конфигурационного пакета
static int _send_conf_packet(const uint8_t * packet)
{
	// Ставим байты по порядку в синхрослове
	static const uint8_t syncword[2] =
	{
			(uint8_t)((UBX_SYNCWORD_VALUE >> 8) & 0xFF),
			(uint8_t)((UBX_SYNCWORD_VALUE >> 0) & 0xFF)
	};

	// Считаем размер тела пакета (без синхрослова и контрольной суммы)
	const uint16_t packet_size = ubx_packet_payload_size(packet) + UBX_HEADER_SIZE;
	const uint16_t crc16 = ubx_packet_checksum(packet, packet_size);
	// Расставляем байты контрольной суммы в правильном порядке
	const uint8_t crc_bytes[2] = {
			ubx_uint16crc_get_crca(crc16),
			ubx_uint16crc_get_crcb(crc16)
	};

	// Отправляем синхрослово
	HAL_StatusTypeDef hal_error;
	hal_error = HAL_UART_Transmit(&uartGPS, (uint8_t*)syncword, sizeof(syncword), HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	// Отправляем сам пакет
	hal_error = HAL_UART_Transmit(&uartGPS, (uint8_t*)packet, packet_size, HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	// Отправляем его контрольную сумму
	hal_error = HAL_UART_Transmit(&uartGPS, (uint8_t*)crc_bytes, sizeof(crc_bytes), HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	return 0;
}



int gps_configure()
{
	// Перебираем все сообщения по одному
	const uint8_t ** packet_ptr = ublox_neo6_cfg_msgs;
	for ( ; *packet_ptr != 0; packet_ptr++)
	{
		const uint8_t * packet = *packet_ptr;
		int last_error = 0;

		// Будем пытаться несколько раз
		for (size_t i = 0; i < ITS_SINS_GPS_CONFIGURE_ATTEMPTS; i++)
		{
			// Встаем в ожидание ACK пакета
			_cfg_status = GPS_CFG_STATUS_WAIT_ACK;
			_cfg_sent_pid = ubx_packet_pid(packet);

			// Засылаем пакет
			_send_conf_packet(packet);

			// так, пакет отправили. Теперь
			// ждем ответа на него

			uint32_t start = HAL_GetTick();
			while(1)
			{
				// Будем полить раз в 100 мс
				HAL_Delay(100);

				// Поллим входщие сообщения
				gps_poll();

				// ничего не получили интересного?
				if (GPS_CFG_STATUS_GOT_ACK == _cfg_status)
				{
					// О, отлично
					last_error = 0;
					break;
				}
				else if (GPS_CFG_STATUS_GOT_NACK == _cfg_status)
				{
					// Не совсем отлично, но тоже неплохо
					// Идем на на следующую попытку
					//int pid = (int)ubx_packet_pid(packet);
					//trace_printf("message 0x%04X rejected\n", pid);
					last_error = -EBADMSG;
					break;
				}
				else
				{
					// Мы все еще ждем. Ну, ждем
					// Если таймаут уже прошел, то штош
					uint64_t time_passed = sins_hal_tick_diff(start, HAL_GetTick());
					if (time_passed > ITS_SINS_GPS_CONFIGURE_TIMEOUT)
					{
						last_error = -ETIMEDOUT;
						break;
					} // if
				} // if as switch
			} // while

			// Если на этой итерации все настроилось, то отлично
			// идем дальше
			if (0 == last_error)
			{
				// int pid = (int)ubx_packet_pid(packet);
				// trace_printf("message 0x%04X accepted\n", pid);
				break;
			}
		}

		if (0 != last_error)
		{
			// Если в процессе что-то пошло не так
			// то завершаемся на этом
			return last_error;
		}
	}

	return 0;
}
