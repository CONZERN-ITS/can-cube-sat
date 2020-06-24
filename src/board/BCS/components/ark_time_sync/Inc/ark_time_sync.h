/*
 * ark_time_sync.h
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef COMPONENTS_ARK_TIME_SYNC_INC_ARK_TIME_SYNC_H_
#define COMPONENTS_ARK_TIME_SYNC_INC_ARK_TIME_SYNC_H_

#include <stdint.h>
#include <stddef.h>

#define ARK_SIGNAL_LENGTH 1000 //ms

#define ARK_TIME_SYNC_PRIOD 50000 //ms

int ark_tsync_send_signal(uint8_t *data, size_t *size);

void ark_tsync_task(void *pvParametres);

#endif /* COMPONENTS_ARK_TIME_SYNC_INC_ARK_TIME_SYNC_H_ */
