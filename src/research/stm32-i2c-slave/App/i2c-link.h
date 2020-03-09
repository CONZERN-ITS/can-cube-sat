#ifndef I2C_INC_H_
#define I2C_INC_H_

#include <stddef.h>

#include "i2c-link-conf.h"

typedef struct i2c_link_stats_t
{
	uint16_t rx_done_cnt;
	uint16_t rx_dropped_cnt;
	uint16_t rx_error_cnt;

	uint16_t tx_done_cnt;
	uint16_t tx_zeroes_cnt;
	uint16_t tx_error_cnt;

	uint16_t restarts_cnt;
	uint16_t listen_done_cnt;
	uint32_t last_error;
} i2c_link_stats_t;


int i2c_link_start(void);

int i2c_link_write(const void * data, size_t data_size);

int i2c_link_read(void * buffer, size_t buffer_size);

void i2c_link_stats(i2c_link_stats_t * statsbuf);

#endif /* I2C_INC_H_ */
