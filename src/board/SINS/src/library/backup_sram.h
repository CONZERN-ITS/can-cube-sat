/*
 * backup_sram.h
 *
 *  Created on: 15 авг. 2020 г.
 *      Author: developer
 */

#ifndef LIBRARY_BACKUP_SRAM_H_
#define LIBRARY_BACKUP_SRAM_H_


#define BACKUP_SRAM_MEMORY_START_ADDRESS 	0x40024000U
#define BACKUP_SRAM_MEMORY_END_ADDRESS		0x40024FFFU


extern int backup_sram_enable_after_reset(void);
extern int backup_sram_enable(void);
extern void backup_sram_write(state_zero_t * data);
extern void backup_sram_read(state_zero_t * data);
extern void backup_sram_erase(void);


#endif /* LIBRARY_BACKUP_SRAM_H_ */
