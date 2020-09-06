/*
 * uplink.c
 *
 *  Created on: Sep 6, 2020
 *      Author: snork
 */

#include "uplink.h"

#include <stm32f1xx_hal.h>
#include "its-i2c-link.h"


extern IWDG_HandleTypeDef hiwdg;


int uplink_packet(const mavlink_message_t * msg)
{
    static uint8_t data[MAVLINK_MAX_PACKET_LEN] = {0};

    uint16_t data_size = mavlink_msg_to_send_buffer(data, msg);
    if (0 == data_size)
        return 0;

    int rc = its_i2c_link_write(data, data_size);
    if (rc > 0)
    {
        // Отправка пакета удалась! сбрасываем вотчдог
        HAL_IWDG_Refresh(&hiwdg);
    }

    return rc;
}
