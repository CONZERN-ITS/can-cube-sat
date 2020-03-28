#include "ark.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int ark_send_cmd(ark_t *hark, ark_cmd_t cmd) {
	return my_i2c_send(hark->i2c_port, hark->address, (uint8_t*) &cmd, 1, hark->timeout);
}

int ark_msg_send(ark_t *hark, uint8_t *data, uint16_t size) {
	esp_err_t err = ark_send_cmd(hark, ARK_CMD_SET_PACKET);
	if (err) {
		printf("Error: send cmd1 = %d\n", (int)err);
		return err;
	}
	//Надо отправить и размер, и пакет. Для этого пришлость воспользоваться низкоуровневыми
	//методами
	i2c_cmd_handle_t cmd =  i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, hark->address << 1 | I2C_MASTER_WRITE, 1);
	i2c_master_write(cmd, (uint8_t*) &size, sizeof(size), 1);
	i2c_master_write(cmd, data, size, 1);
	i2c_master_stop(cmd);
	err = i2c_master_cmd_begin(hark->i2c_port, cmd, hark->timeout / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if (err) {
		printf("Error: send cmd2 = %d\n", (int)err);
		return err;
	}

	return 0;
}

int ark_msg_recieve(ark_t *hark, uint8_t *data, uint16_t size) {

	int err = ark_send_cmd(hark, ARK_CMD_GET_SIZE);
	if (err) {
		printf("Error: send cmd1 = %d\n", (int)err);
		return err;
	}


	uint16_t rsize = 0;
	err = my_i2c_recieve(hark->i2c_port, hark->address, (uint8_t*) &rsize, sizeof(rsize), hark->timeout);
	if (err) {
		printf("Error: send cmd2 = %d\n", (int)err);
		return err;
	}
	if (rsize > size || rsize == 0) {
		printf("Bad size: %d\n", rsize);
		return 0;
	}
	size = rsize;

	err = ark_send_cmd(hark, ARK_CMD_GET_PACKET);
	if (err) {
		printf("Error: send cmd3 = %d\n", (int)err);
		return err;
	}

	err = my_i2c_recieve(hark->i2c_port, hark->address, data ,size, hark->timeout);
	if (err) {
		printf("Error: send cmd4 = %d\n", (int)err);
		return err;
	}
	return 0;
}
