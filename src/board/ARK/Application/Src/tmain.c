#include <ina219_helper.h>
#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
#include "i2c.h"
#include "its-i2c-link.h"
#include "mavlink/its/mavlink.h"
#include "battery.h"
#include "mavlink_system_id.h"


#define MSG_TEMP_DELAY 1000
#define MSG_INA_DELAY 1000

enum TEMP_STATE {
    DS_PROCESSING,
    DS_READY
} ;

int tmain(void) {

    its_i2c_link_start();

    //----------------ds18b20 init--------------------
    struct BatteryBatch hbb;
    ds18b20_config_t hds[BATTERY_CNT_BAT_IN_ROW][BATTERY_CNT_ROW_IN_PACK];
    hds[0][0].rom = 0x0;
    //hds[i][k].rom = 0xXXXXXXXXXXX;

    hbb.batteryRow[0].battery[0].hds = &hds[0][0];
    //hbb.batteryRow[j].battery[m] = &hds[i][k];

    onewire_t how;
    onewire_Init(&how, GPIOB, GPIO_PIN_0);
    HAL_Delay(100);

    for (int i = 0; i < BATTERY_CNT_BAT_IN_ROW; i++) {
        for (int j = 0; j < BATTERY_CNT_ROW_IN_PACK; j++) {

            hds[i][j].how = &how;
            hds[i][j].resolution = ds18b20_Resolution_12bits;
            ds18b20_SetResolution(&hds[i][j], hds[i][j].resolution);
        }
    }
    ds18b20_StartAll(&hds[0][0]);

    //--------------------ina init-----------------------------
    ina219_t hina[2];

    hbb.ina_bat = &hina[0];
    hbb.ina_bus = &hina[1];

    ina219_init_default(&hina[0], &hi2c2, INA219_I2CADDR_A1_GND_A0_GND << 1);
    ina219_init_default(&hina[1], &hi2c2, INA219_I2CADDR_A1_VSP_A0_VSP << 1);
    //--------------------ad527x init------------------------

    struct ad527x_t had[2];

    hbb.dp_charge = &had[0];
    hbb.dp_discharge = &had[1];

    ad527x_init(&had[0], AD527x_050, &hi2c2, AD527X_I2CADDR_ADDR_VDD << 1);
    ad527x_init(&had[1], AD527x_050, &hi2c2, AD527X_I2CADDR_ADDR_GND << 1);

    //--------------------bms init--------------------------
    struct bms_t hbms[BATTERY_CNT_ROW_IN_PACK];

    for (int i = 0; i < BATTERY_CNT_ROW_IN_PACK; i++) {
        hbb.batteryRow[i].hbms = &hbms[i];
    }

    bms_init(&hbms[0], GPIOA, GPIO_PIN_1);
    //bms_init(&hbms[1], GPIOA, GPIO_PIN_2);
    //bms_init(&hbms[2], GPIOA, GPIO_PIN_3);
    //bms_init(&hbms[3], GPIOA, GPIO_PIN_4);
    //bms_init(&hbms[4], GPIOA, GPIO_PIN_5);
    //bms_init(&hbms[5], GPIOA, GPIO_PIN_6);

    battery_init_bus_voltage(&hbb);

    printf("while start\n");

    uint32_t prev_ds = gettime().ms;
    uint32_t prev_ina = gettime().ms;

    enum TEMP_STATE state_t = DS_PROCESSING;

    while(1) {
        uint32_t now = gettime().ms;

        //---------------------------INA219 read-----------------------------
        {
            ina219_secondary_data_t sd;
            ina219_primary_data_t pd;

            ina219_read_all(hbb.ina_bus, &pd, &sd);
            hbb.busCurrent = ina219_current_convert(hbb.ina_bus, sd.current);
            hbb.busVoltage = ina219_bus_voltage_convert(hbb.ina_bus, pd.busv);

            ina219_read_all(hbb.ina_bat, &pd, &sd);
            hbb.batteryCurrent = ina219_current_convert(hbb.ina_bat, sd.current);
            hbb.batteryVoltage = ina219_bus_voltage_convert(hbb.ina_bat, pd.busv);
        }
        battery_update_dpd_voltage(&hbb);
        battery_update_dpc_voltage(&hbb);

        if (now - prev_ina > MSG_INA_DELAY) {
            prev_ina = now;
            for (int i = 0; i < BATTERY_CNT_ROW_IN_PACK; i++) {
                for (int j = 0; j < BATTERY_CNT_BAT_IN_ROW; j++) {
                    mavlink_message_t msg;
                    mavlink_electrical_state_t st = {
                            .area_id = i * BATTERY_CNT_BAT_IN_ROW + j,
                            .current = hbb.busVoltage,//FIXME Не хватает полей для данных электричества
                            .voltage = hbb.busCurrent,
                            .time_boot_ms = gettime().ms
                    };
                    mavlink_msg_electrical_state_encode(MAVLINK_SYS_ID_KA, MAVLINK_COMP_ID_ARK, &msg, &st);
                    its_i2c_link_write(&msg, sizeof(msg));
                }
            }
        }

        //---------------------------DS18B20 read-----------------------------
        now = gettime().ms;
        {
            if (onewire_ReadBit(hbb.batteryRow[0].battery[0].hds->how) && state_t == DS_PROCESSING) {

                for (int i = 0; i < BATTERY_CNT_ROW_IN_PACK; i++) {
                    for (int j = 0; j < BATTERY_CNT_BAT_IN_ROW; j++) {
                        ds18b20_Read(hbb.batteryRow[i].battery[j].hds, &hbb.batteryRow[i].battery[j].temperature);
                    }
                }
                ds18b20_StartAll(hbb.batteryRow[0].battery[0].hds);
                state_t = DS_READY;
            }

        }

        if (now - prev_ds > MSG_TEMP_DELAY && state_t == DS_READY) {
            prev_ds = now;
            state_t = DS_PROCESSING;


            for (int i = 0; i < BATTERY_CNT_ROW_IN_PACK; i++) {
                for (int j = 0; j < BATTERY_CNT_BAT_IN_ROW; j++) {
                    mavlink_message_t msg;
                    mavlink_termal_state_t st = {
                            .area_id = i * BATTERY_CNT_BAT_IN_ROW + j,
                            .temperature = hbb.batteryRow[i].battery[j].temperature,
                            .time_boot_ms = now
                    };
                    mavlink_msg_termal_state_encode(MAVLINK_SYS_ID_KA, MAVLINK_COMP_ID_ARK, &msg, &st);
                    its_i2c_link_write(&msg, sizeof(msg));
                }
            }
        }


    }

    return 0;
}
