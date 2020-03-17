#include "main.h"

enum ad527x_i2c_address {
    AD527X_I2CADDR_ADDR_VDD = 0b101100,
    AD527X_I2CADDR_ADDR_GND = 0b101111,
    AD527X_I2CADDR_ADDR_NC = 0b101110,
};

struct ad527x_cfg {
    int max_pos;
    int kohms;
    int shift;
};

enum ad527x_type {
    AD527x_020,
    AD527x_050,
    AD527x_100,
    AD5274_020,
    AD5274_100,
};

struct ad527x_t {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    struct ad527x_cfg cfg;
    uint8_t buf[2];
};

void ad527x_init(struct ad527x_t *had, enum ad527x_type type, I2C_HandleTypeDef *hi2c, uint8_t address);

int ad527x_setResistaneRaw(struct ad527x_t *had, uint16_t val);

int ad527x_setResistane(struct ad527x_t *had, float val);
