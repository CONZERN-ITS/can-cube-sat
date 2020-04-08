/*
 * time_util.c
 *
 *  Created on: Apr 4, 2020
 *      Author: snork
 */

#include "time_util.h"

#include <assert.h>
#include <errno.h>




//! Перевод времени GPS в UNIX время
inline struct timeval gps_time_to_unix_time(uint16_t week, uint32_t tow_ms)
{
	struct timeval rv;
	rv.tv_sec = week * 60 * 60 * 24 * 7 + tow_ms / 1000;
	rv.tv_usec = (tow_ms % 1000) * 1000;

	rv.tv_sec += 315964800; // эпоха GPS в UNIX времени
	return rv;
}

