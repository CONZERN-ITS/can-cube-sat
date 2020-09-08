/*
 * backup_sram.h
 *
 *  Created on: 15 авг. 2020 г.
 *      Author: developer
 */

#ifndef LIBRARY_BACKUP_SRAM_H_
#define LIBRARY_BACKUP_SRAM_H_

#include "state.h"


#define BACKUP_SRAM_MEMORY_START_ADDRESS 	0x40024000U
#define BACKUP_SRAM_MEMORY_END_ADDRESS		0x40024FFFU


int backup_sram_enable_after_reset(void);
int backup_sram_enable(void);

void backup_sram_read_reset_counter(uint8_t * counter);
void backup_sram_write_reset_counter(uint8_t * counter);
void backup_sram_read_zero_state(state_zero_t * data);
void backup_sram_write_zero_state(state_zero_t * data);

void backup_sram_erase(void);


#endif /* LIBRARY_BACKUP_SRAM_H_ */
