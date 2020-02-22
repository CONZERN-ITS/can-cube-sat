#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
#define DS18B20_COUNT 1

int tmain(void) {

    printf("tmain start\n");


    onewire_t how = {0};
    //onewire_pin_init(&how, GPIOB, GPIO_PIN_10);
    onewire_Init(&how, GPIOB, GPIO_PIN_0);

    onewire_Reset(&how);

    HAL_Delay(100);

    ds18b20_config_t hds[DS18B20_COUNT];
    int ds_count = 0;


    hds[0].rom = 0;
    onewire_ReadRom(&how, &hds[0].rom);
    hds[0].resolution = ds18b20_Resolution_12bits;
    hds[0].how = &how;
    ds18b20_SetResolution(&hds[0], hds[0].resolution);
    ds18b20_Start(&hds[0]);

    printf("while start\n");
    while(1) {
        float t = 0.1;
        if (ds18b20_Read(&hds[0], &t)) {
            printf("temp: %0.2f\n", t);
            ds18b20_Start(&hds[0]);
        }

    }
    volatile int index = 0;

    while (1) {
        printf("Count of 't': %d\n", index);
        index += 2;
    }

    return 0;
}
