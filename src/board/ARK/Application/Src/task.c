/*
 * task.c
 *
 *  Created on: Apr 24, 2020
 *      Author: sereshotes
 */

#include "task.h"

#include "assert.h"


typedef struct {
    task_t task;
    int is_used;
    int is_inited;
} task_private_t;


static task_private_t task_array[TASK_MAX_TCOUNT];
static int count;
static task_id cur_task;

void task_create(task_t task, task_id *tid) {
    assert(count < TASK_MAX_TCOUNT);

    if (tid) {
        *tid = count;
    }
    task_array[count].task = task;
    task_array[count].is_used = 1;
    (*task.init)(task.arg);
    task_array[count].is_inited = 1;
    count++;
}

void task_init_all(void) {
    for (cur_task = 0; cur_task < count; cur_task++) {
        task_private_t *tp = &task_array[cur_task];

        if (tp->is_used && !tp->is_inited) {
            (*tp->task.init)(tp->task.arg);
            tp->is_inited = 1;
        }
    }

}
void task_update_all(void) {
    for (cur_task = 0; cur_task < count; cur_task++) {
        task_private_t *tp = &task_array[cur_task];

        if (tp->is_used) {
            (*tp->task.update)(tp->task.arg);
        }
    }

}
void task_delete(task_id tid) {
    task_array[tid].is_used = 0;
}
void task_delete_this(void) {
    task_array[cur_task].is_used = 0;
}
