/*
 * internet.h
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#ifndef COMPONENTS_INTERNET_INC_INTERNET_H_
#define COMPONENTS_INTERNET_INC_INTERNET_H_


#define IP2_OUR_IP "192.168.31.40"
#define IP2_OUR_PORT 51690
void my_sntp_client_init();

void my_sntp_server_init();

void task_socket_comm(void *pvParameters);

void task_socket_recv(void *arg);

void wifi_init_sta(void);

void wifi_init_ap(void);

void ntp_server_task(void *arg);


#endif /* COMPONENTS_INTERNET_INC_INTERNET_H_ */
