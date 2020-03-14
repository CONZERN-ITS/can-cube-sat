#include <ina219_helper.h>
#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
#include "i2c.h"
#include "its-i2c-link.h"
#include "mavlink/its/mavlink.h"
#include "data_ds18b20.h"
#include "data_ina219.h"

int tmain(void) {

    its_i2c_link_start();

    data_ds18b20_init();
    data_ina219_init();


    printf("while start\n");

    uint32_t prev = gettime().ms;
    while(1) {
        uint32_t now = gettime().ms;
        if (now - prev > 1000) {
            data_ds18b20_read();
            data_ina219_read();

            data_ds18b20_send();
            data_ina219_send();
        }

    }

    return 0;
}
