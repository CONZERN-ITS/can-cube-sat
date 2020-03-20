#include "process_ad527x.h"
#include "data_ina219.h"
#include "ad527x.h"
#include "main.h"

#define AD527X_COUNT BATTERY_PACK_COUNT * 2

static struct ad527x_t had[AD527X_COUNT];
static uint16_t res[AD527X_COUNT];

const float THR_BELOW = 6;
const float THR_UP = 8;

#define AD527X_ADDRESS 0

void process_ad527x_init() {
    ad527x_init(&had[0], AD527x_050, &hi2c2, AD527X_ADDRESS);
    res[0] = 10;
}
void process_ad527x_update() {
    for (int i = 0; i < AD527X_COUNT; i++) {
        res[i] = (ina219_voltage[i] - THR_BELOW) * had[i].cfg.max_pos / (THR_UP - THR_BELOW);
        ad527x_setResistaneRaw(&had[i], res[i]);
    }

}
