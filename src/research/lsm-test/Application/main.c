/*
 * main.c
 *
 *  Created on: Mar 1, 2020
 *      Author: snork
 */

#include <stdio.h>

#include "lsm6ds3.h"


int user_main()
{

	printf("reboot\n");
	lsm6ds3_init();

	return 0;
}
