/*
 * input_op.c
 *
 *  Created on: Aug 25, 2020
 *      Author: sereshotes
 */
#include "input_op.h"

#include <string.h>

#include "esp_log.h"

#include "init_helper.h"
#include "shift_reg.h"
#include "control_heat.h"
#include "control_magnet.h"
#include "control_vcc.h"
#include "radio.h"
#include "sdio.h"


static int _sr_control(char *arg);

static int _sr_set(char *arg);

static int _radio_send(char *arg);

static int _sdio(char *arg);

void input_init(op_handler_t *hop) {
	op_add_op(hop, "sr_control",_sr_control);
	op_add_op(hop, "sr_set",_sr_set);
	op_add_op(hop, "radio",_radio_send);
	op_add_op(hop, "sd",_sdio);
}

extern shift_reg_handler_t hsr;
#define TOS(a) #a
static int _sr_control(char *arg) {
	char c;
	char name[20];
	int mode;

	sscanf(arg, "%c %20s %d", &c, name, &mode);
	int name2;
	if (strcmp(TOS(ITS_BSK_1), name) == 0) {
		name2 = ITS_BSK_1;
	}
	if (strcmp(TOS(ITS_BSK_2), name) == 0) {
		name2 = ITS_BSK_2;
	}
	if (strcmp(TOS(ITS_BSK_2A), name) == 0) {
		name2 = ITS_BSK_2A;
	}
	if (strcmp(TOS(ITS_BSK_3), name) == 0) {
		name2 = ITS_BSK_3;
	}
	if (strcmp(TOS(ITS_BSK_4), name) == 0) {
		name2 = ITS_BSK_4;
	}
	if (strcmp(TOS(ITS_BSK_5), name) == 0) {
		name2 = ITS_BSK_5;
	}
	int test = 1;
	switch (c) {
	case 'v':
		control_vcc_bsk_enable(name2, mode);
		break;
	case 'h':
		control_heat_bsk_enable(name2, mode);
		break;
	case 'm':
		control_magnet_enable(name2, mode);
		break;
	default:
		test = 0;
	}
	ESP_LOGD("INPUT", "char: %c name: %d mode: %d", c, name2, mode);
	if (test) {
		ESP_LOGD("INPUT", "0x%X 0x%X 0x%X", hsr.byte_arr[0], hsr.byte_arr[1], hsr.byte_arr[2]);
		return shift_reg_load(&hsr);
	} else {
		ESP_LOGE("INPUT", "no");
		return -1;
	}
}


static int _sr_set(char *arg) {
	uint32_t t = 0;
	int total_offset = 0;
	int offset = 0;
	for (int i = 0; i < hsr.arr_size && sscanf(arg + total_offset, " 0x%X%n", &t, &offset) == 1; i++) {
		hsr.byte_arr[i] = t;
		total_offset += offset;
	}
	shift_reg_load(&hsr);
	ESP_LOGD("INPUT", "0x%X 0x%X 0x%X", hsr.byte_arr[0], hsr.byte_arr[1], hsr.byte_arr[2]);
	return 0;
}

static int _radio_send(char *arg) {
	char *ptr = arg;
	while (!isspace(*ptr)) {
		ptr++;
	}
	*ptr = 0;
	if (strcmp(arg, "pause") == 0) {
		radio_send_suspend();
	} else if (strcmp(arg, "resume") == 0) {
		radio_send_resume();
	}
	return 0;
}

static int _sdio(char *arg) {
	char *ptr = arg;
	while (!isspace(*ptr)) {
		ptr++;
	}
	*ptr = 0;
	if (strcmp(arg, "pause") == 0) {
		sd_suspend();
	} else if (strcmp(arg, "resume") == 0) {
		sd_resume();
	} else {
		return -1;
	}
	return 0;
}
