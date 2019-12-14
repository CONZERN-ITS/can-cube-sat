/*
 * the_main.c
 *
 *  Created on: 3 июл. 2019 г.
 *      Author: snork
 */


#include "main.h"

#include "bme280.h"
#include "can.h"
#include "ds18b20.h"
#include "sensors.h"



int the_main(void)
{


    bme280_register_spi(&hbme, &hspi1, BME_CS_GPIO_Port, BME_CS_Pin, 1000);
    bme280_init(&hbme);


    onewire_t wire;
    onewire_Init(&wire, GPIOB, GPIO_PIN_0);
    hds.how = &wire;
    hds.resolution = ds18b20_Resolution_12bits;
    hds.rom = 0x00;
    onewire_ReadRom(&wire, &hds.rom);

    ds18b20_StartAll(&hds);

    ahrs_system_init();

    while(1)
    {
        sensors_bme280_update();
        sensors_external_update();

        //camera_system_update(&hcam);
    }

    return 0;
}


