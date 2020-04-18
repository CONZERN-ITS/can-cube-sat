/*
 * internet.h
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef COMPONENTS_INTERNET_INC_INTERNET_H_
#define COMPONENTS_INTERNET_INC_INTERNET_H_


void my_sntp_init();

void task_socket_comm(void *pvParameters);

void wifi_init_sta(void);


#endif /* COMPONENTS_INTERNET_INC_INTERNET_H_ */
