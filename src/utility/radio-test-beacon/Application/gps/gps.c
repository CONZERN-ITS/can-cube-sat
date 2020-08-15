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
#include <stm32f1xx.h>

#define GPS_CYCLOBUFFER_SIZE 1024
#define GPS_LINBUFFER_SIZE 256
#define GPS_CFG_ATTEMPTS 10
#define GPS_MAX_POLL_SIZE 256
#define GPS_CONFIGURE_TIMEOUT 1000

// ==========================================
// Это для фонового приёма пакетиков
// ==========================================

//! Хендл для уарта для GPS
extern UART_HandleTypeDef huart2;
static UART_HandleTypeDef * uartGPS = &huart2;

//! Циклобуфер для входящих байт GPS
static uint8_t _uart_cycle_buffer[1024];
static int _uart_cycle_buffer_head;
static int _uart_cycle_buffer_tail;

//! Линейный буфер для сбора сообщений потоковым парсером UBX
static uint8_t _ubx_sparser_buffer[256];
//! Контекст потокового парсера UBX
static ubx_sparser_ctx_t _sparser_ctx;

//! Колбек пользователя. Мы устанавливаем свой, но пользователю тоже нужно данные
static ubx_sparser_packet_callback_t _user_packet_callback;

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

static int sins_hal_status_to_errno(HAL_StatusTypeDef h_status);
static uint64_t sins_hal_tick_diff(uint32_t start, uint32_t stop);


//! Колбек для входящих GPS пакетов.
/*! Некоторые мы будем перехватывать для уточнения службы времени */
void _internal_packet_callback(void * user_arg, const ubx_any_packet_t * packet_)
{
	// FIXME: Возможно стоит что-то сделать, чтобы не использовать оба пакета на одном такте?
	// Если они оба придут (а они оба придут)
	// Возможно стоит выключить TIMTP в целом?
	switch (packet_->pid)
	{
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


void USART2_IRQHandler(void)
{
	//Проверка флага о приеме байтика по USART
	if ((USART2->SR & USART_SR_RXNE) != 0)
	{
		//Сохранение принятого байтика
		volatile uint8_t tmp = USART2->DR;

		int next_head = _uart_cycle_buffer_head + 1;
		if (next_head >= GPS_CYCLOBUFFER_SIZE)
			next_head = 0;

		if (next_head == _uart_cycle_buffer_tail)
			return; // Переполнение циклобуфера. Выкидываем байт

		_uart_cycle_buffer[_uart_cycle_buffer_head] = tmp;
		_uart_cycle_buffer_head = next_head;
	}
}


static uint64_t sins_hal_tick_diff(uint32_t start, uint32_t stop)
{
	if (stop >= start)
		return stop - start;
	else
		return (uint64_t)start + 0xFFFFFFFF - stop;
}


static int sins_hal_status_to_errno(HAL_StatusTypeDef h_status)
{
	int rc;

	switch (h_status)
	{
	case HAL_OK:
		rc = 0;
		break;

	case HAL_BUSY:
		rc = -EBUSY;
		break;

	case HAL_TIMEOUT:
		rc = -ETIMEDOUT;
		break;

	default:
	case HAL_ERROR:
		rc = -EFAULT;
		break;
	}

	return rc;
}


static int _init_uart()
{
	//Включение прерывания USART: RXNE
	__HAL_UART_ENABLE_IT(uartGPS, UART_IT_RXNE);

	// nvic в hal_msp
	return 0;
}


int gps_init(
		gps_packet_callback_t packet_callback, void * packet_callback_arg
)
{
	// Настриваем потоковый парсер UBX пакетов
	_user_packet_callback = packet_callback;
	ubx_sparser_reset(&_sparser_ctx);
	ubx_sparser_set_pbuffer(&_sparser_ctx, _ubx_sparser_buffer, sizeof(_ubx_sparser_buffer));
	ubx_sparser_set_packet_callback(&_sparser_ctx, _internal_packet_callback, packet_callback_arg);

	// Настраиваем циклобуфер для уарта
	_uart_cycle_buffer_head = 0;
	_uart_cycle_buffer_tail = 0;

	// разрешаем прерывания уарта
	_init_uart();

	// Все готово! поехали!
	return 0;
}


int gps_flush(void)
{
	_uart_cycle_buffer_tail = _uart_cycle_buffer_head;
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
		if (next_tail >= GPS_CYCLOBUFFER_SIZE)
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
extern const uint8_t * ublox_neo7_cfg_msgs[];


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
	hal_error = HAL_UART_Transmit(uartGPS, (uint8_t*)syncword, sizeof(syncword), HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	// Отправляем сам пакет
	hal_error = HAL_UART_Transmit(uartGPS, (uint8_t*)packet, packet_size, HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	// Отправляем его контрольную сумму
	hal_error = HAL_UART_Transmit(uartGPS, (uint8_t*)crc_bytes, sizeof(crc_bytes), HAL_MAX_DELAY);
	if (hal_error != HAL_OK)
		return sins_hal_status_to_errno(hal_error);

	return 0;
}



int gps_configure()
{
	// Перебираем все сообщения по одному
	const uint8_t ** packet_ptr = ublox_neo7_cfg_msgs;
	for ( ; *packet_ptr != 0; packet_ptr++)
	{
		const uint8_t * packet = *packet_ptr;
		int last_error = 0;

		// Будем пытаться несколько раз
		for (size_t i = 0; i < GPS_CFG_ATTEMPTS; i++)
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
					if (time_passed > GPS_CONFIGURE_TIMEOUT)
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

