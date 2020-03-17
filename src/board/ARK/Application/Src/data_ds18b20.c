#include "data_ds18b20.h"

#include "main.h"

#include "ds18b20.h"
#include "mavlink/its/mavlink.h"
#include "mavlink_system_id.h"
#include "Time.h"
#include "i2c.h"
#include "its-i2c-link.h"

#define DS18B20_COUNT BATTERY_PACK_COUNT * BATTERY_COUNT_IN_PACK

static onewire_t how = {0};
static ds18b20_config_t hds[DS18B20_COUNT];
static int ds_count = 0;
static float temp[DS18B20_COUNT];

enum {
    DS18B20_FINISH,
    DS18B20_COMPUTING
} ds18b20_state;

void data_ds18b20_init() {
    onewire_Init(&how, GPIOB, GPIO_PIN_0);
    HAL_Delay(100);

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
    ds18b20_StartAll(&hds[0]);
    ds18b20_state = DS18B20_COMPUTING;

}

void data_ds18b20_read() {
    if (onewire_ReadBit(hds->how)) {
        ds18b20_state = DS18B20_FINISH;
       for (int i = 0; i < ds_count; i++) {
           ds18b20_Read(&hds[i], &temp[i]);
       }
       ds18b20_StartAll(&hds[0]);
   }
}

void data_ds18b20_send() {
    if (ds18b20_state != DS18B20_FINISH)
        return;

    for (int i = 0; i < DS18B20_COUNT; i++) {
        mavlink_message_t msg;
        mavlink_termal_state_t st = {
                .area_id = i,
                .temperature = temp[i],
                .time_boot_ms = gettime().ms
        };
        mavlink_msg_termal_state_encode(MAVLINK_SYS_ID_KA, MAVLINK_COMP_ID_ARK, &msg, &st);
        its_i2c_link_write(&msg, sizeof(msg));
    }
    ds18b20_state = DS18B20_COMPUTING;

}
