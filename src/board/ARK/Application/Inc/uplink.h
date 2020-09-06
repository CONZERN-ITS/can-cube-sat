/*
 * uplink.h
 *
 *  Created on: Sep 6, 2020
 *      Author: snork
 */

#ifndef INC_UPLINK_H_
#define INC_UPLINK_H_

#include "mavlink_help2.h"

//! Функция отпавляет пакет в БКУ
/*! Возвращаемое значение аналогично тому, что возвращает its_i2c_link_write */
int uplink_packet(const mavlink_message_t * msg);

#endif /* INC_UPLINK_H_ */
