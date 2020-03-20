/*
 * battery.h
 *
 *  Created on: 15 мар. 2020 г.
 *      Author: sereshotes
 */

#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_
#include "battery_config.h"

#include "ds18b20.h"
#include "ina219.h"
#include "ad527x.h"
#include "bms.h"


/*
 * Batteries in module have specific connection.
 * Here is example for 2x6
 * []-[]-|
 * []-[]-|
 * []-[]-|
 * []-[]-|
 * []-[]-|
 * []-[]-|
 *
 * Every battery has ds18b20 to get temperature of
 * all batteries and switch some of them off.
 */


struct Battery {
    ds18b20_config_t *hds;
    float temperature;
};

struct BatteryRow {
    struct Battery battery[BATTERY_CNT_BAT_IN_ROW];
    struct bms_t *hbms;

    enum BATTERY_TEMP_STATE{
        BAT_TEMP_NORMAL,
        //BAT_COLD,
        BAT_TEMP_OVERHEAT
    } batteryRowTempState;
    enum {
        BAT_ROW_OFF,
        BAT_ROW_ON
    } batteryRowState;
};

struct BatteryBatch {
    struct BatteryRow batteryRow[BATTERY_CNT_ROW_IN_PACK];
    struct ad527x_t *dp_charge;
    struct ad527x_t *dp_discharge;
    ina219_t *ina_bus;
    ina219_t *ina_bat;

    float busVoltage;
    float batteryVoltage;
    float busCurrent;
    float batteryCurrent;
    enum {
        BAT_IDLE,
        BAT_CHARGE,
        BAT_DISCHARGE,
        BAT_ERROR
    } batterySystemState;

};


int battery_update_dpd_voltage(struct BatteryBatch * hbb);

int battery_update_dpc_voltage(struct BatteryBatch * hbb);

int battery_init_bus_voltage(struct BatteryBatch * hbb);

int battery_update_row_state(struct BatteryRow *bat);


#endif /* INC_BATTERY_H_ */
