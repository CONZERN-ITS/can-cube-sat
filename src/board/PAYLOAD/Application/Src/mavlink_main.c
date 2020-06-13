/*
 * mavlink_impl.c
 *
 *  Created on: Jun 14, 2020
 *      Author: snork
 */

#include "mavlink_main.h"

#include <errno.h>

#include "its-i2c-link-conf.h"
#include <its-i2c-link.h>

mavlink_system_t mavlink_system = {CUBE_1, CUBE_1_PL};


void mavlink_main_send_to_its_link(mavlink_channel_t channel, const uint8_t * buffer, uint16_t buffer_size)
{
	(void)channel;
	its_i2c_link_write(buffer, buffer_size);
}


int mavlink_main_get_packet(mavlink_message_t * msg)
{
	uint8_t in_buffer[I2C_LINK_PACKET_SIZE];
	int rcved_size = its_i2c_link_read(in_buffer, sizeof(in_buffer));
	if (rcved_size < 0)
		return rcved_size;

	// о, что-то пришло
	// Считаем что у нас не больше одного мав сообщения на один i2c-link пакет
	for (int i = 0; i < rcved_size; i++)
	{
		mavlink_status_t status;
		uint8_t byte = in_buffer[i];
		int parsed = mavlink_parse_char(MAVLINK_COMM_0, byte, msg, &status);
		if (parsed)
			return 0;
	}

	return -EAGAIN;
}
