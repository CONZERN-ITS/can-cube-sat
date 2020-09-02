/*
 * operator_ip.c
 *
 *  Created on: Aug 22, 2020
 *      Author: sereshotes
 */
#include "operator_ip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task.h"
#include "esp_bit_defs.h"
#include "esp_sntp.h"
#include "esp_netif.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwipopts.h"

static char *TAG = "OP_IP";


static int op_init_ip(op_ip_t *hop);

static int op_recieve_cmd_ip(op_ip_t *hop, char *str, int size);

static op_cmd_t *op_find_cmd_ip(op_ip_t *hop, char *name);

static void op_add_op_ip(op_ip_t *hop, char *name, int (*func)(char *arg));

void op_config_ip(op_ip_t *hop, uint16_t port) {
	hop->hop.operations.recieve_cmd = op_recieve_cmd_ip;
	hop->hop.operations.init = op_init_ip;
	hop->hop.operations.find_cmd = op_find_cmd_ip;
	hop->hop.operations.add_op = op_add_op_ip;

	hop->port = port;
	hop->arr_size = 0;
	hop->arr_max_size = OP_IP_ARRAY_MAX_SIZE;
	hop->sin = -1;
}

static int op_init_ip(op_ip_t *hop) {
	hop->sin = socket(AF_INET, SOCK_DGRAM, 0);
	if (hop->sin < 0) {
		ESP_LOGE(TAG, "can't create socket");
		return -1;
	}
	struct sockaddr_in addr = {0};
	addr.sin_port = htons(hop->port);
	addr.sin_family = AF_INET;
	if (bind(hop->sin, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(hop->sin);
		ESP_LOGE(TAG, "can't bind socket");
		return -1;
	}

	return 0;
}

static int op_recieve_cmd_ip(op_ip_t *hop, char *str, int size) {
	return recvfrom(hop->sin, str, size, 0, 0, 0);
}

static op_cmd_t *op_find_cmd_ip(op_ip_t *hop, char *name) {
	for (int i = 0; i < hop->arr_size; i++) {
		if (strcmp(hop->cmd_array[i].name, name) == 0) {
			return &hop->cmd_array[i];
		}
	}
	return 0;
}

static void op_add_op_ip(op_ip_t *hop, char *name, int (*func)(char *arg)){
	assert(strlen(name) < OP_MAX_CMD_NAME_SIZE);
	assert(hop->arr_size < hop->arr_max_size);

	strncpy(hop->cmd_array[hop->arr_size].name, name, OP_MAX_CMD_NAME_SIZE);
	hop->cmd_array[hop->arr_size].func = func;
	hop->arr_size++;

}
