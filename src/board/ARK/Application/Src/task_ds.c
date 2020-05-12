/*
 * task_read.c
 *
 *  Created on: Apr 24, 2020
 *      Author: sereshotes
 */
#include <task_ds.h>
#include "main.h"

#include "assert.h"
#include "task.h"
#include "ds18b20.h"

static float ds_temp[TDS_TEMP_MAX_COUNT];
static int _is_valid[TDS_TEMP_MAX_COUNT];

static ds18b20_config_t hds[TDS_TEMP_MAX_COUNT];
static int ds_count;
static onewire_t how;



static void (*callback_arr[TDS_CALLBACK_COUNT])(void);
static int callback_count;

void tds18b20_add_callback(void (*f)(void)) {
    assert(callback_count < TDS_CALLBACK_COUNT);
    callback_arr[callback_count++] = f;
}

int tds18b20_get_value(float **arr, int **is_valid) {
    *arr = ds_temp;
    *is_valid = _is_valid;
    return ds_count;
}

void _ds_sort(ds18b20_config_t *arr, int size) {
    int is_sorted = 0;
    for (int j = size - 1; j > 0 && !is_sorted; j--) {
        is_sorted = 1;
        for (int i = 0; i < j; i++) {
            if (arr[i].rom > arr[i + 1].rom) {
                is_sorted = 0;

                ds18b20_config_t t = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = t;
            }
        }
    }
}

void task_ds_init(void *arg) {
    onewire_Init(&how, OW_GPIO_Port, OW_Pin);
    HAL_Delay(100);

    int status = onewire_First(&how);
    int index = 0;
    while (status) {
        //Save ROM number from device
        onewire_GetFullROM(&how, &hds[index].rom);
        hds[index].how = &how;
        hds[index].resolution = ds18b20_Resolution_12bits;
        index++;
        //Check for new device
        status = onewire_Next(&how);

        if (index >= TDS_TEMP_MAX_COUNT) {
            break;
        }
    }
    ds_count = index;

    if (ds_count == 0) {
        return;
    }
    _ds_sort(hds, ds_count);
    ds18b20_StartAll(&hds[0]);
}

void task_ds_update(void *arg) {
    static uint32_t prev = 0;
    if (ds_count == 0 || HAL_GetTick() - prev < TDS_READ_PERIOD || !ds18b20_AllDone(hds)) {
        return;
    }

    prev += TDS_READ_PERIOD;
    for (int i = 0; i < ds_count; i++) {
        _is_valid[i] = ds18b20_Read(&hds[i], &ds_temp[i]);
    }
    ds18b20_StartAll(&hds[0]);

    for (int i = 0; i < callback_count; i++) {
        (*callback_arr[i])();
    }
}


