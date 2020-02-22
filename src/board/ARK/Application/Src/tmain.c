#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
#define DS18B20_COUNT 2

int tmain(void) {

    printf("tmain start\n");


    onewire_t how = {0};
    //onewire_pin_init(&how, GPIOB, GPIO_PIN_10);
    onewire_Init(&how, GPIOB, GPIO_PIN_0);

    onewire_Reset(&how);

    HAL_Delay(100);

    ds18b20_config_t hds[DS18B20_COUNT];
    int ds_count = 0;


    onewire_ResetSearch(&how);
    for (ds_count = 0; ds_count < DS18B20_COUNT; ds_count++) {
        if (!onewire_Next(&how)) {
            break;
        }

        onewire_GetFullROM(&how, &hds[ds_count].rom);
        hds[ds_count].how = &how;
        hds[ds_count].resolution = ds18b20_Resolution_12bits;
        ds18b20_SetResolution(&hds[ds_count], hds[ds_count].resolution);
    }
    printf("DS count: %d\n", ds_count);

    ds18b20_StartAll(&hds[0]);

    printf("while start\n");
    while(1) {
        float t = 0.1;

        if (onewire_ReadBit(hds->how)) {
            for (int i = 0; i < ds_count; i++) {
                ds18b20_Read(&hds[i], &t);
                printf("[%d]: %0.2f\t", i, t);
            }
            printf("\n");
            ds18b20_StartAll(&hds[0]);
        }

    }
    volatile int index = 0;

    while (1) {
        printf("Count of 't': %d\n", index);
        index += 2;
    }

    return 0;
}
