/*
 * my_i2c.c
 *
 *  Created on: 27 мар. 2020 г.
 *      Author: sereshotes
 */

#include "my_i2c.h"

int my_i2c_send(i2c_port_t i2c_port, uint8_t address, uint8_t *data, int size, int timeout) {
	if (size <= 0) {
		printf("Error size: size <= 0");
		return -1;
	}
	esp_err_t err;
	i2c_cmd_handle_t cmd =  i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, 1);
	i2c_master_write(cmd, data, size, 1);
	i2c_master_stop(cmd);

	err = i2c_master_cmd_begin(i2c_port, cmd, timeout);

	i2c_cmd_link_delete(cmd);
	return err;
}
int my_i2c_recieve(i2c_port_t i2c_port, uint8_t address, uint8_t *data, int size, int timeout) {
	esp_err_t err;
	i2c_cmd_handle_t cmd =  i2c_cmd_link_create();
	if (size <= 0) {
		return -1;
	}
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_READ, 1);
	i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);

	err = i2c_master_cmd_begin(i2c_port, cmd, timeout);
	i2c_cmd_link_delete(cmd);
	return err;
}
