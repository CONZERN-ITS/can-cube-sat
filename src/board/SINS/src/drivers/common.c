/*
 * common.c
 *
 *  Created on: 8 апр. 2020 г.
 *      Author: snork
 */

#include "common.h"

#include <errno.h>


int sins_hal_status_to_errno(HAL_StatusTypeDef h_status)
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


uint64_t sins_hal_tick_diff(uint32_t start, uint32_t stop)
{
	if (stop >= start)
		return stop - start;
	else
		return (uint64_t)start + 0xFFFFFFFF - stop;
}
