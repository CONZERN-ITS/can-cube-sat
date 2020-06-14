/*
 * time_svc.c
 *
 *  Created on: Jun 12, 2020
 *      Author: snork
 */


#include "time_svc.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

// Целые части секунд
static time_t _seconds;


// Запоминаем метку времени в которое мы получили последний
// синхроимпульс
// используем двойную буферизацию для борьбы с внезваными прерываниями
static struct timeval _last_sync_tmv_1;
static struct timeval _last_sync_tmv_2;
static struct timeval * _last_sync_tmv_inner = &_last_sync_tmv_1;
static struct timeval * _last_sync_tmv_outer = &_last_sync_tmv_2;

// Ожидается ли
static bool _sync_pending = false;

// Проверка - есть ли ожидающий свапа таймштамп
static bool _check_swap_sync()
{
	if (!_sync_pending)
		return false;

	__disable_irq();
	struct timeval * tmp = _last_sync_tmv_inner;
	_last_sync_tmv_inner = _last_sync_tmv_outer;
	_last_sync_tmv_outer = tmp;
	_sync_pending = false;
	__enable_irq();

	return true;
}


void time_svc_init(void)
{
	// Просто включаем прерывания таймера и включаем его
	__HAL_TIM_ENABLE_IT(TIMESVC_TIM_HANDLE, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE(TIMESVC_TIM_HANDLE);

	// Таймер настроен так, что тикает два раза в секунду
	// и переполняется раз в секунду.
	// (частота на входе таймера 2 кГц, период 2000 циклов)
}


void time_svc_gettimeofday(struct timeval * tmv)
{
	time_t seconds, seconds2;
	suseconds_t subseconds;

	seconds = _seconds;
	subseconds = __HAL_TIM_GET_COUNTER(TIMESVC_TIM_HANDLE);
	seconds2 = _seconds;

	if (seconds != seconds2 && (subseconds & 0xFF00) == 0)
	{
		// Это значит что после того как мы прочитали переполнения
		// таймер переполнился и мы взяли значение счетчика для следующей итерации таймера
		// поэтому берем правильное значение переполнений
		seconds = seconds2;
	}

	tmv->tv_sec = seconds;
	tmv->tv_usec = 1000 * (subseconds / 2) + 500 * (subseconds % 2);
}


void time_svc_settimeofday(const struct timeval * tmv)
{
	// Останавливаем таймер
	__HAL_TIM_DISABLE(TIMESVC_TIM_HANDLE);

	// Ставим количество секунд
	_seconds = tmv->tv_sec;

	// Ставим счетчик таймеру на количество половинок секунд
	// (в миллисекунды из микросекунд и в половинки
	const uint16_t cnt = (tmv->tv_usec / 1000) * 2;
	__HAL_TIM_SET_COUNTER(TIMESVC_TIM_HANDLE, cnt);

	// Запускаем таймер
	__HAL_TIM_ENABLE(TIMESVC_TIM_HANDLE);
}


void time_svc_on_tim_interrupt(void)
{
	_seconds++;
}


void time_svc_on_sync_interrupt(void)
{
	time_svc_gettimeofday(_last_sync_tmv_outer);
	_sync_pending = true;
}


void time_svc_on_mav_message(const mavlink_message_t * msg)
{
	// Нас интересуют только такие сообщения
	if (MAVLINK_MSG_ID_TIMESTAMP != msg->msgid)
		return;

	// Мы не ждали такого сообщения
	if (!_check_swap_sync())
		return;

	const struct timeval * local_sync_stamp = _last_sync_tmv_inner;

	// Смотрим сколько времени было у хоста, когда он бросал нам sync метку
	const struct timeval host_sync_stamp = {
		.tv_sec = mavlink_msg_timestamp_get_time_s(msg),
		.tv_usec = mavlink_msg_timestamp_get_time_us(msg)
	};

	// Считаем разницу между временем у нас и у хоста на момент метки
	struct timeval diff;
	diff.tv_sec = host_sync_stamp.tv_sec - local_sync_stamp->tv_sec;
	diff.tv_usec = host_sync_stamp.tv_usec - local_sync_stamp->tv_usec;
	// нормируем
	if (diff.tv_usec < 0)
	{
		diff.tv_sec -= 1;
		diff.tv_usec += 1000 * 1000;
	}

	// Если разница меньше половины миллисекунды, то забиваем конечно
	if (diff.tv_sec == 0 && abs(diff.tv_usec) < 500)
		return;

	// Берем наше текущее время
	struct timeval current_time;
	time_svc_gettimeofday(&current_time);

	// добавляем разницу
	current_time.tv_sec += diff.tv_sec;
	current_time.tv_usec += diff.tv_usec;

	if (current_time.tv_usec > 1000 * 1000)
	{
		current_time.tv_sec += 1;
		current_time.tv_usec -= 1000 * 1000;
	}

	// Делаем это время текущим
	// FIXME: нужно бы добавить сколько-то микросекунд на все эти операции
	time_svc_settimeofday(&current_time);
}
