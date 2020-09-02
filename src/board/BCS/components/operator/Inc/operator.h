/*
 * operator.h
 *
 *  Created on: Aug 22, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_OPERATOR_INC_OPERATOR_H_
#define COMPONENTS_OPERATOR_INC_OPERATOR_H_


#define OP_MAX_CMD_SIZE 60
#define OP_MAX_CMD_NAME_SIZE 20
#define OP_MAX_ARG_SIZE ((OP_MAX_CMD_SIZE - OP_MAX_CMD_NAME_SIZE))

typedef struct {
	char name[OP_MAX_CMD_NAME_SIZE];
	int (*func)(char *arg);
	char *arg;
} op_cmd_t;


struct op_handler_t;

typedef struct {
	int (*init)(struct op_handler_t *hop);
	op_cmd_t *(*find_cmd)(struct op_handler_t *hop, char *name);
	int (*recieve_cmd)(struct op_handler_t *hop, char *str, int size);
	void (*add_op)(struct op_handler_t *hop, char *name, int (*func)(char *arg));
} op_operations_t;

typedef struct op_handler_t {
	op_operations_t operations;
	void *cmd_storage;
} op_handler_t;

int op_init(struct op_handler_t *hop);
void op_add_op(struct op_handler_t *hop, char *name, int (*func)(char *arg));

#endif /* COMPONENTS_OPERATOR_INC_OPERATOR_H_ */
