/*
 * task.c
 *
 *  Created on: Apr 24, 2020
 *      Author: sereshotes
 */

#include "task.h"

#include "main.h"

#include "stdio.h"
#include "string.h"
#include "assert.h"

typedef struct {
    task_t task;
    int is_used;
    int is_inited;
} task_private_t;


static task_private_t task_array[TASK_MAX_TCOUNT];
static int count = 0;
static task_id cur_task = 0;

void task_create(task_t task, task_id *tid) {
    assert(count < TASK_MAX_TCOUNT);

    if (tid) {
        *tid = count;
    }
    task_private_t *tp = &task_array[count];
    count++;
    memcpy(&tp->task, &task, sizeof(task));
    tp->is_used = 1;
    (*task.init)(task.arg);
    tp->is_inited = 1;
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
#if TASK_DEBUG
            uint32_t start = HAL_GetTick();
#endif
            (*tp->task.update)(tp->task.arg);
#if TASK_DEBUG
            printf("TASK: task %s update time is: %d\n",
                    tp->task.name, (int)(HAL_GetTick() - start));
#endif
        }
    }

}
void task_delete(task_id tid) {
    task_array[tid].is_used = 0;
}
void task_delete_this(void) {
    task_array[cur_task].is_used = 0;
}
