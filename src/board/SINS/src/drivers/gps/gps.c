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
#include <diag/Trace.h>

#include "sins_config.h"
#include "../common.h"
#include "../time_svc/time_util.h"
#include "../time_svc/time_svc.h"

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

typedef struct gps_cfg_state_t
{
	//! Включен ли вообще режим конфигурации
	int enabled;

	//! Указатель на текущий конфигурационный пакет
	const uint8_t ** packet_ptr;

	//! Время отправки очередного пакета конфигурации
	uint32_t sent_packet_timestamp;
	//! Счетчик попыток отправки очередного пакета
	int sent_packet_attempt;

	//! Идентификатор отправленного пакета конфигурацииs
	ubx_pid_t sent_packet_pid;
	//! Статус ожидания ответа на конфигурационный пакет
	enum {
		GPS_CFG_ACK_STATUS_IDLE = 0,
		GPS_CFG_ACK_STATUS_WAIT_ACK,
		GPS_CFG_ACK_STATUS_GOT_ACK,
		GPS_CFG_ACK_STATUS_GOT_NACK
	} sent_packet_ack_status;

	//! Результат конфигурации
	int last_error;

} gps_cfg_state_t;

//! Глобальное состояние модуля конфигурации
static gps_cfg_state_t _cfg_state = {0};

//! Набор сообщений конфигурации приёмника
//extern const uint8_t * ublox_neo7_cfg_msgs[];
extern const uint8_t * ublox_neo6_cfg_msgs[];

static void _internal_packet_callback(void * user_arg, const ubx_any_packet_t * packet_);
static int _init_uart(void);
static void _init_pps(void);
static void _gps_configure_step(void);
static int _gps_configure_step_packet(void);


void NONHAL_PPS_MspInit(void);


//! Колбек для входящих GPS пакетов.
/*! Некоторые мы будем перехватывать для уточнения службы времени
    И для конфигурации */
static void _internal_packet_callback(void * user_arg, const ubx_any_packet_t * packet_)
{
	gps_cfg_state_t * const cfg_state = &_cfg_state;

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
		if (cfg_state->enabled)
		{
			// дождалис
			const ubx_ack_packet_t * packet = &packet_->packet.ack;
			if (cfg_state->sent_packet_pid == packet->packet_pid)
				cfg_state->sent_packet_ack_status = GPS_CFG_ACK_STATUS_GOT_ACK;
		}
		break;

	// Это сообщение используется для конфигурации
	case UBX_PID_CFG_NACK:
		if (cfg_state->enabled)
		{
			// не дождались
			const ubx_nack_packet_t * packet = &packet_->packet.nack;
			if (cfg_state->sent_packet_pid == packet->packet_pid)
				cfg_state->sent_packet_ack_status = GPS_CFG_ACK_STATUS_GOT_NACK;
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
		time_svc_world_set_time(_next_pps_time, TIME_SVC_TIMEBASE__GPS);


	HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
//	if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_9))
//		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, RESET);
//	else
//		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, SET);

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


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


//! Проход конечного автомата при оотправке очередного пакета
/*! Выполняется в нескольких состояниях, поэтому вынесен в отдельную функцию */
static int _gps_configure_step_packet()
{
	gps_cfg_state_t * state = &_cfg_state;
	const uint8_t * packet = *state->packet_ptr;

	if (NULL == packet)
	{
		// Мы дошли до последнего пакета и успешно завершились
		state->last_error = 0;
		state->enabled = 0;
		return 0;
	}

	// Если количество попыток исчерпано - завершаемся
	if (state->sent_packet_attempt > ITS_SINS_GPS_CONFIGURE_ATTEMPTS)
	{
		state->enabled = 0;
		// state->result выставили при прошлой ошибке
		return state->last_error;
	}

	int rc = _send_conf_packet(*state->packet_ptr);
	if (0 != rc)
	{
		// Если уже даже отправить не можем - тут же сворачиваемся
		state->enabled = 0;
		state->last_error = -ENOENT;
		return rc;
	}

	state->sent_packet_ack_status = GPS_CFG_ACK_STATUS_WAIT_ACK;
	state->sent_packet_pid = ubx_packet_pid(packet);
	state->sent_packet_timestamp = HAL_GetTick();
	state->sent_packet_attempt++;
	return 0;
}


//! Выполнить очередной шаго конфигурации GPS
static void _gps_configure_step()
{
	gps_cfg_state_t * state = &_cfg_state;

	if (!state->enabled)
		return;

	switch (state->sent_packet_ack_status)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
	case GPS_CFG_ACK_STATUS_GOT_ACK:
		// Отлично, переходим к следующему пакету
		state->packet_ptr++;
#pragma GCC diagnostic pop
		/* no break */

	case GPS_CFG_ACK_STATUS_IDLE:
		state->sent_packet_attempt = 0;
		// Если нет - отправляем следующий
		_gps_configure_step_packet();
		break;

	case GPS_CFG_ACK_STATUS_GOT_NACK:
		// Ставим ошибку
		state->last_error = -EBADMSG;
		// Пробуем отправить пакет еще раз, если попытки не кончилисьs
		_gps_configure_step_packet();
		break;

	case GPS_CFG_ACK_STATUS_WAIT_ACK:
		// Ответ еще не пришел, проверяем таймаут
		{
			uint32_t now = HAL_GetTick();
			if (now - state->sent_packet_timestamp > ITS_SINS_GPS_CONFIGURE_TIMEOUT)
			{
				// Ставим ошибку
				state->last_error = -ETIMEDOUT;
				// Таймаут наступил, пробуем отправить пакет еще раз, если попытки не кончились
				_gps_configure_step_packet();
			}
			// Продолжаем ждать
		}
		break;
	}

}



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

	// Настраиваем железо для работы с PPS
	_init_pps();

	// Настраиваем уартовое железо
	int error =_init_uart();
	if (error != 0)
		return error;

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

	_gps_configure_step();
	return i;
}


void gps_configure_begin()
{
	gps_cfg_state_t * state = &_cfg_state;

	if (state->enabled)
		return; // Мы уже конфигурируемся

	state->packet_ptr = ublox_neo6_cfg_msgs;
	state->sent_packet_ack_status = GPS_CFG_ACK_STATUS_IDLE;

	state->enabled = 1;
}


int gps_configure_status()
{
	gps_cfg_state_t * state = &_cfg_state;

	if (state->enabled)
		return -EWOULDBLOCK;

	return state->last_error;
}


