#include "data_ina219.h"

#include "main.h"

#include "ina219_helper.h"
#include "mavlink/its/mavlink.h"
#include "mavlink_system_id.h"
#include "Time.h"
#include "i2c.h"
#include "its-i2c-link.h"

static ina219_t hina[INA219_COUNT] = { 0 };
float ina219_voltage[INA219_COUNT];
float ina219_current[INA219_COUNT];

enum {
    DS18B20_FINISH,
    DS18B20_COMPUTING
} ds18b20_state[INA219_COUNT];

void data_ina219_init() {
    ina219_init_default(&hina[0], &hi2c2, INA219_I2CADDR_A1_GND_A0_GND << 1);
}

void data_ina219_read() {
    for (int i = 0; i < INA219_COUNT; i++) {
        ina219_secondary_data_t sd;
        ina219_primary_data_t pd;
        ina219_read_all(&hina[i], &pd, &sd);
        //_ina_read(&hina, &current, &power);
        ina219_current[i] = ina219_current_convert(&hina[i], sd.current);
        ina219_voltage[i] = ina219_bus_voltage_convert(&hina[i], pd.busv);
    }
}

void data_ina219_send() {

    for (int i = 0; i < INA219_COUNT; i++) {
        mavlink_message_t msg;
        mavlink_electrical_state_t st = {
                .area_id = i,
                .current = ina219_current[i],
                .voltage = ina219_voltage[i],
                .time_boot_ms = gettime().ms
        };
        mavlink_msg_electrical_state_encode(MAVLINK_SYS_ID_KA, MAVLINK_COMP_ID_ARK, &msg, &st);
        its_i2c_link_write(&msg, sizeof(msg));
    }
}
