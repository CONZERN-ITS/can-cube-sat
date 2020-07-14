/*
 * syscalls.c
 *
 *  Created on: 14 июл. 2020 г.
 *      Author: developer
 */

#include <assert.h>


int
_write(int file, char* ptr, int len)
{
	assert(0);
	return 0;
}

int
_close(int fildes)
{
	assert(0);
	return -1;
}

int
_read(int file __attribute__((unused)), char* ptr __attribute__((unused)),
    int len __attribute__((unused)))
{
	assert(0);
	return -1;
}

int _lseek(int file, int ptr, int dir)
{
	assert(0);
	return 0;
}
