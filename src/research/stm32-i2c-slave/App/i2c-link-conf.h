#ifndef I2C_LINK_CONF_H_
#define I2C_LINK_CONF_H_

#include <stm32f1xx_hal.h>

#include "main.h"

#define I2C_LINK_PACKET_SIZE (279)
#define I2C_LINK_RXBUF_COUNT (5)
#define I2C_LINK_TXBUF_COUNT (5)

extern I2C_HandleTypeDef hi2c1;

#define I2C_LINK_BUS_HANDLE (&hi2c1)

#endif /* I2C_LINK_CONF_H_ */
