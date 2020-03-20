#include "battery.h"




int _battery_set_dp_voltage(struct ad527x_t *hdp, float voltage, float v_bot, float v_top) {
    uint16_t t = (voltage - v_bot) * hdp->cfg.max_pos / (v_top - v_bot);
    return ad527x_setResistaneRaw(hdp, t);
}


int battery_update_dpd_voltage(struct BatteryBatch * hbb) {
    if (hbb->busVoltage > BATTERY_DPD_BAT_MAX) {
        hbb->busVoltage = BATTERY_DPD_BAT_MAX;
    }
    if (hbb->busVoltage < BATTERY_DPD_BAT_MIN) {
        hbb->busVoltage = BATTERY_DPD_BAT_MIN;
    }

    return _battery_set_dp_voltage(hbb->dp_discharge, hbb->busVoltage, BATTERY_DPD_BAT_MIN, BATTERY_DPD_BAT_MAX);
}

int battery_update_dpc_voltage(struct BatteryBatch * hbb) {

    return 0;
}

int battery_init_bus_voltage(struct BatteryBatch * hbb) {
    battery_update_dpc_voltage(hbb);
    battery_update_dpd_voltage(hbb);
}

int battery_update_row_state(struct BatteryRow *bat) {

    switch (bat->batteryRowTempState) {
        case BAT_TEMP_NORMAL: {
            int t = 0;
            for (int i = 0; i < BATTERY_CNT_BAT_IN_ROW; i++) {
                if (bat->battery[i].temperature > BATTERY_TEMP_TOP_THD) {
                    t = 1;
                    break;
                }
            }
            if (t) {
                bat->batteryRowTempState = BAT_TEMP_OVERHEAT;
                bms_off(bat->hbms);
                bat->batteryRowState = BAT_ROW_OFF;
            }
            break;
        }

        case BAT_TEMP_OVERHEAT: {
            int t = 0;
            for (int i = 0; i < BATTERY_CNT_BAT_IN_ROW; i++) {
                if (bat->battery[i].temperature > BATTERY_TEMP_BOT_THD) {
                    t = 1;
                    break;
                }
            }
            if (!t) {
                bat->batteryRowTempState = BAT_TEMP_NORMAL;
                bms_on(bat->hbms);
                bat->batteryRowState = BAT_ROW_ON;
            }
            break;
        }
    }

}
