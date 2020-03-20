/*
 * AD527x.c
 *
 *  Created on: 3 янв. 2020 г.
 *      Author: sereshotes
 */

#include "ad527x.h"

#define  AD527X_CMD_RDAC_WR  1
#define  AD527X_CMD_RESET    4
#define  AD527X_CMD_CTL_WR   7

#define  AD527X_CTL_RDAC_WR_EN   0b0010

#define AD527X_TIMEOUT 500


static const struct ad527x_cfg ad527x_cfg[] = {
    [AD527x_020] = { .max_pos = 1024, .kohms = 20 },
    [AD527x_050] = { .max_pos = 1024, .kohms = 50 },
    [AD527x_100] = { .max_pos = 1024, .kohms = 100 },
    [AD5274_020] = { .max_pos = 256,  .kohms = 20,  .shift = 2 },
    [AD5274_100] = { .max_pos = 256,  .kohms = 100, .shift = 2 },
};


static int ad527x_sendCmd(struct ad527x_t *had, uint8_t cmd, uint16_t val, uint32_t timeout)
{
    uint8_t buf[2];

    //Sent bytes must look like 00CCCCXX XXXXXXXX, where C - command, X - data value
    buf[0] = (cmd << 2) | ((val >> 8) & 0x3);
    buf[1] = (uint8_t) val;

    return HAL_I2C_Master_Transmit(had->hi2c, had->address, buf, sizeof(buf), timeout);
}

void ad527x_init(struct ad527x_t *had, enum ad527x_type type, I2C_HandleTypeDef *hi2c, uint8_t address) {
    had->address = address;
    had->hi2c = hi2c;
    had->cfg = ad527x_cfg[type];
    ad527x_sendCmd(had, AD527X_CMD_RESET, 0, AD527X_TIMEOUT);
    ad527x_sendCmd(had, AD527X_CMD_CTL_WR, AD527X_CTL_RDAC_WR_EN, AD527X_TIMEOUT);
}

int ad527x_setResistaneRaw(struct ad527x_t *had, uint16_t val) {
    return ad527x_sendCmd(had, AD527X_CMD_RDAC_WR, val, AD527X_TIMEOUT);
}
int ad527x_setResistane(struct ad527x_t *had, float val) {
    if (val > had->cfg.kohms * 1000) {
        val = had->cfg.kohms * 1000;
    }
    uint16_t res = (val * had->cfg.max_pos) / (had->cfg.kohms * 1000);
    return ad527x_setResistaneRaw(had, res);
}




