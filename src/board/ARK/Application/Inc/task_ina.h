/*
 * task_ina.h
 *
 *  Created on: Apr 25, 2020
 *      Author: sereshotes
 */

#ifndef INC_TASK_INA_H_
#define INC_TASK_INA_H_

#define TINA_READ_PERIOD 0
#define TINA_CALLBACK_COUNT 3
#define INA_TIMEOUT 100

#if defined CUBE_1 && !defined CUBE_2
#   define TINA_COUNT 1
#elif defined CUBE_2 && !defined CUBE_1
#   define TINA_COUNT 0
#else
#   error "invalid cube definition"
#endif

typedef struct {
    float current;
    float voltage;
} tina_value_t;


int tina219_get_value(tina_value_t **arr, int **is_valid);

void tina219_add_ina_callback(void (*f)(void));

void task_ina_init(void *arg);

void task_ina_update(void *arg);

#endif /* INC_TASK_INA_H_ */
