/*
 * backup_sram.c
 *
 *  Created on: 15 авг. 2020 г.
 *      Author: developer
 */

#include <stm32f4xx_hal.h>

#include "string.h"

#include "state.h"
#include "backup_sram.h"


int backup_sram_enable_after_reset()
{
	// Enable PWR clock
	RCC->AHB1ENR |= RCC_APB1ENR_PWREN;

	//
	PWR->CR |= PWR_CR_DBP;

	//Enable backup SRAM clock
	RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;

	return 0;
}


int backup_sram_enable()
{
	backup_sram_enable_after_reset();

	//Enable backup regulator
	PWR->CSR |= PWR_CSR_BRE;

	//Wait backup regulator ready flag
	while (!(PWR->CSR & (PWR_CSR_BRR)));

	return 0;
}


void backup_sram_write(state_zero_t * data)
{
	memcpy((void*)BACKUP_SRAM_MEMORY_START_ADDRESS, (void*)data, sizeof(state_zero_t));
}


void backup_sram_read(state_zero_t * data)
{
	memcpy((void*)data, (void*)BACKUP_SRAM_MEMORY_START_ADDRESS, sizeof(state_zero_t));
}
