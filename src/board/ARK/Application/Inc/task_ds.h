/*
 * task_read.h
 *
 *  Created on: Apr 25, 2020
 *      Author: sereshotes
 */

#ifndef INC_TASK_DS_H_
#define INC_TASK_DS_H_


#define TDS_TEMP_MAX_COUNT 10
#define TDS_READ_PERIOD 1000
#define TDS_CALLBACK_COUNT 3


void tds18b20_add_callback(void (*f)(void));

int tds18b20_get_value(float **arr, int **is_valid);

void task_ds_init(void *arg);

void task_ds_update(void *arg);

#endif /* INC_TASK_DS_H_ */
