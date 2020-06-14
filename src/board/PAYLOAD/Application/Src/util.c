/*
 * util.c
 *
 *  Created on: May 17, 2020
 *      Author: snork
 */

#include "util.h"


#include <errno.h>


int hal_status_to_errno(HAL_StatusTypeDef status)
{
	switch (status)
	{
	case HAL_OK:
		return 0;
	case HAL_TIMEOUT:
		return -ETIMEDOUT;
	case HAL_BUSY:
		return -EBUSY;
	case HAL_ERROR:
		return -EFAULT;

	default:
		return -ENOSYS;
	}
}
