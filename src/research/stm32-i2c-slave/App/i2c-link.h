#ifndef I2C_INC_H_
#define I2C_INC_H_

#include <stddef.h>

#include "i2c-link-conf.h"

int i2c_link_start(void);

int i2c_link_write(const void * data, size_t data_size);

int i2c_link_read(void * buffer, size_t buffer_size);

#endif /* I2C_INC_H_ */
