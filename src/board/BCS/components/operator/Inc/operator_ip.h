/*
 * operator_ip.h
 *
 *  Created on: Aug 25, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_OPERATOR_INC_OPERATOR_IP_H_
#define COMPONENTS_OPERATOR_INC_OPERATOR_IP_H_


#include "operator.h"

#include <inttypes.h>

#define OP_IP_ARRAY_MAX_SIZE 10

typedef struct {
	op_handler_t hop;
	op_cmd_t cmd_array[OP_IP_ARRAY_MAX_SIZE];
	int arr_size;
	int arr_max_size;
	int sin;
	uint16_t port;
} op_ip_t;

void op_config_ip(op_ip_t *hop, uint16_t port);

#endif /* COMPONENTS_OPERATOR_INC_OPERATOR_IP_H_ */
