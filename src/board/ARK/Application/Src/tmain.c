#include <ina219_helper.h>
#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
#include "i2c.h"


int tmain(void) {

    printf("tmain start\n");

    ina219_t hina = {0};
    ina219_init_default(&hina, &hi2c1, INA219_I2CADDR_A1_GND_A0_GND << 1);

    printf("while start\n");
    while(1) {
        float current = 0, power = 0;
        ina219_secondary_data_t sd;
        ina219_primary_data_t pd;
        ina219_read_all(&hina, &pd, &sd);
        //_ina_read(&hina, &current, &power);
        float cur = ina219_current_convert(&hina, sd.current);
        float pow = ina219_power_convert(&hina, sd.power);
        float vbs = ina219_bus_voltage_convert(&hina, pd.busv);
        float vsh = ina219_shunt_voltage_convert(&hina, pd.shuntv);
        printf("I: %0.2f  Us: %0.2f  U: %0.2f  P: %0.2f\n", cur * 1000, vsh, vbs * 1000 , pow * 1000);

    }

    return 0;
}
