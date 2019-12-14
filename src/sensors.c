/*
 * thread.c
 *
 *  Created on: 10 мая 2019 г.
 *      Author: sereshotes
 */

#include "main.h"

#include "bme280.h"
#include "ds18b20.h"
#include "can.h"


#define SENSORS_SEND_FREQ 2

struct bme280_dev_s hbme;
ds18b20_config_t hds;

//Read data from BME280 sensor and send scaled_pressure MAVLink message
void sensors_bme280_update(void)
{
    struct bme280_float_data_s data;
	bme280_read(&hbme, (char*)&data, sizeof(struct bme280_float_data_s));

}

//Read data from DS18B20 and MPX2100AP sensors and send scaled_pressure2 MAVLink message
void sensors_external_update(void)
{
	static float temperature = 0;

    if (ds18b20_AllDone(&hds) && ds18b20_Read(&hds, &temperature))
    {
        ds18b20_StartAll(&hds);
	}
}


