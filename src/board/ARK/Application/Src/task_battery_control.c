/*
 * task_battery_control.c
 *
 *  Created on: May 20, 2020
 *      Author: sereshotes
 */
#include "battery_config.h"
#include "task_battery_control.h"

#include "main.h"
#include "ad527x.h"
#include "ina219_helper.h"
#include "ds18b20.h"

#include "task.h"
#include "task_ina.h"
#include "task_ds.h"
#include "mavlink/its/mavlink.h"



#define LAPPEND(x,y) x ## y
#define APPEND(x,y) LAPPEND(x,y)

extern I2C_HandleTypeDef hi2c2;

struct ad527x_t had_in;
struct ad527x_t had_out;

enum BAT_STATE {
    NORMAL,
    OVERHEATED,
    DEAD
};
enum BAT_STATE bstate[BATTERY_MAX_ROW_COUNT];

typedef struct {
    enum BAT_STATE state;
    uint32_t last_working[BATTERY_IN_ROW];
    float temp[BATTERY_IN_ROW];
} battery_row_t;
battery_row_t bat_row[BATTERY_MAX_ROW_COUNT];

int _battery_set_dp_voltage(struct ad527x_t *hdp, float voltage, float v_bot, float v_top) {
    uint16_t t = (voltage - v_bot) * hdp->cfg.max_pos / (v_top - v_bot);
    return ad527x_setResistaneRaw(hdp, t);
}
void task_battery_control_init(void *arg) {
    ad527x_init(&had_in, AD527x_050, &hi2c2, AD527X_I2CADDR_ADDR_VDD << 1);
    ad527x_init(&had_out, AD527x_050, &hi2c2, AD527X_I2CADDR_ADDR_GND << 1);
    int rc = ad527x_setResistaneRaw(&had_out, 100);
    printf("rc = %d\n", rc);
    for (int i = 0; i < BATTERY_MAX_ROW_COUNT; i++) {
        for (int j = 0; j < BATTERY_IN_ROW; j++) {
            bat_row[i].last_working[j] = 0;
        }
    }
}

static void update_temp(battery_row_t *bat, int size) {
    float *temp = 0;
    int *valid = 0;
    int count = tds18b20_get_value(&temp, &valid);

    uint32_t now = HAL_GetTick();
    int i = 0;
    for (; i < count; i++) {
        if (valid[i]) {
            bat[i / BATTERY_IN_ROW].last_working[i % BATTERY_IN_ROW] = now;
            bat[i / BATTERY_IN_ROW].temp[i % BATTERY_IN_ROW] = temp[i];
        } else {
            bat[i / BATTERY_IN_ROW].temp[i % BATTERY_IN_ROW] = NAN;
            bat[i / BATTERY_IN_ROW].last_working[i % BATTERY_IN_ROW] = now - 2 * BATTERY_DEAD_TIMEOUT;
        }
    }
    for (; i < size * BATTERY_IN_ROW; i++) {
        bat[i / BATTERY_IN_ROW].last_working[i % BATTERY_IN_ROW] = now - 2 * BATTERY_DEAD_TIMEOUT;
    }
}

static void update_state(battery_row_t *bat) {
    uint32_t now = HAL_GetTick();

    if (bat->state == DEAD) {
        bat->state = NORMAL;
    }
    for (int i = 0; i < BATTERY_IN_ROW; i++) {
        if (bat->last_working[i] - now > BATTERY_DEAD_TIMEOUT) {
            bat->state = DEAD;
        }
    }

    switch (bat->state) {
    case NORMAL: {
        for (int i = 0; i < BATTERY_IN_ROW; i++) {
            if (!isnan(bat->temp[i])) {
                if (bat->temp[i] > BATTERY_THD_NORMAL_TO_OVERHEATED) {
                    bat->state = OVERHEATED;
                }
            }
        }
    }
    case OVERHEATED: {
        bat->state = NORMAL;
        for (int i = 0; i < BATTERY_IN_ROW; i++) {
            if (!isnan(bat->temp[i])) {
                if (bat->temp[i] > BATTERY_THD_OVERHEATED_TO_NORMAL) {
                    bat->state = OVERHEATED;
                }
            }
        }
    }
    case DEAD: {
    }
    }
}


int battery_update_dpd_voltage(float bus_voltage) {
    if (bus_voltage > BATTERY_DPD_BAT_MAX) {
        bus_voltage = BATTERY_DPD_BAT_MAX;
    }
    if (bus_voltage < BATTERY_DPD_BAT_MIN) {
        bus_voltage = BATTERY_DPD_BAT_MIN;
    }

    return _battery_set_dp_voltage(&had_out, bus_voltage, BATTERY_DPD_BAT_MIN, BATTERY_DPD_BAT_MAX);
}
void update_dcdc(void) {

    tina_value_t *t = 0;
    int *valid = 0;
    if (tina219_get_value(&t, &valid) >= 2) {
        if (valid[BATTERY_INA_BUS]) {
            battery_update_dpd_voltage(t[BATTERY_INA_BUS].voltage);
        }
    }

}

void task_battery_control_update(void *arg) {
    return;
    update_temp(bat_row, BATTERY_MAX_ROW_COUNT);
    for (int i = 0; i < BATTERY_MAX_ROW_COUNT; i++) {
        update_state(&bat_row[i]);
        HAL_GPIO_WritePin(BATTERY_BASE_PORT, BATTERY_BASE_PIN + i, bat_row[i].state == NORMAL);
    }
    update_dcdc();


}
