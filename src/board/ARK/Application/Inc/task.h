/*
 * task.h
 *
 *  Created on: Apr 24, 2020
 *      Author: sereshotes
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_


#define TASK_MAX_PRIO 2
#define TASK_MAX_TCOUNT 10
#define TASK_DEBUG 0
#define TASK_NAME_MAX_LENGTH 20

typedef struct {
    void (*init)(void *arg);
    void (*update)(void *arg);
    void *arg;
    char name[TASK_NAME_MAX_LENGTH];
} task_t;

typedef int task_id;


void task_create(task_t task, task_id *tid);

void task_init_all(void);

void task_update_all(void);

void task_delete(task_id tid);

void task_delete_this(void);


#endif /* INC_TASK_H_ */
