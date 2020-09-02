/*
 * operator.c
 *
 *  Created on: Aug 22, 2020
 *      Author: sereshotes
 */

#include "operator.h"


#include <assert.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

static char *TAG = "OP";

#define OP_RETRY_DELAY 100 //ms

static int op_execute(struct op_handler_t *hop, op_cmd_t* cmd) {
	return cmd->func(cmd->arg);
}

#define _NTOS(a) #a
#define NTOS(a) _NTOS(a)

void op_task(void *arg) {
	struct op_handler_t *hop = arg;
	while (1) {
		char str[OP_MAX_CMD_SIZE] = {0};
		int rc;
		op_cmd_t *cmd = 0;
		ESP_LOGD(TAG, "wait for cmd");
		int size = hop->operations.recieve_cmd(hop, str, sizeof(str));
		ESP_LOGD(TAG, "got cmd");
		if (size < 0) {
			vTaskDelay(OP_RETRY_DELAY);
			continue;
		}

		char name[OP_MAX_CMD_NAME_SIZE + 1] = {0};
		rc = sscanf(str, "%"NTOS(OP_MAX_CMD_NAME_SIZE)"s", name);
		if (rc != 1) {
			ESP_LOGD(TAG, "command too long");
			continue;
		}

		cmd = hop->operations.find_cmd(hop, name);
		if (!cmd) {
			ESP_LOGD(TAG, "no such command: %s", name);
			continue;
		}
		cmd->arg = str + strlen(name) + 1;
		rc = op_execute(hop, cmd);
		if (!rc) {
			ESP_LOGD(TAG, "op %s is success", name);
		} else {
			ESP_LOGD(TAG, "op %s is not success", name);
		}
	}
}

int op_init(struct op_handler_t *hop) {
	int rc;

	rc = hop->operations.init(hop);
	if (!rc) {
		rc = xTaskCreate(op_task, "Op task", 4000, hop, 1, 0);
		return pdPASS != rc;
	}
	return rc;
}


void op_add_op(struct op_handler_t *hop, char *name, int (*func)(char *arg)) {
	hop->operations.add_op(hop, name, func);
}
