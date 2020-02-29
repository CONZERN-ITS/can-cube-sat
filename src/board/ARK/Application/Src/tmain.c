#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
#include "ina_helper.h"
#include "i2c.h"


int tmain(void) {

    printf("tmain start\n");

    ina219_t hina = {0};
    _ina_init(&hina, &hi2c1, INA219_I2CADDR_A1_GND_A0_GND << 1);

    printf("while start\n");
    while(1) {
        float current = 0, power = 0;
        ina219_secondary_data_t sd;
        ina219_primary_data_t pd;
        ina219_read_all(&hina, &pd, &sd);
        //_ina_read(&hina, &current, &power);
        float cur = _ina_current_convert(&hina, sd.current);
        float pow = _ina_power_convert(&hina, sd.power);
        float vbs = _ina_bus_voltage_convert(&hina, pd.busv);
        float vsh = _ina_shunt_voltage_convert(&hina, pd.shuntv);
        printf("I: %0.2f  Us: %0.2f  U: %0.2f  P: %0.2f\n", cur * 1000, vsh, vbs * 1000 , pow * 1000);

    }

    return 0;
}
