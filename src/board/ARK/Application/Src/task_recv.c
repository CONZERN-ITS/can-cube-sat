/*
 * task_recv.c
 *
 *  Created on: Apr 28, 2020
 *      Author: sereshotes
 */
#include "task_recv.h"

#include "main.h"

#include "task.h"
#include "its-i2c-link.h"
#include "assert.h"


void task_recv_init(void *arg) {

}

#define TRECV_MAX_CALLBACK_COUNT 5

static void (*trecv_callback_arr[TRECV_MAX_CALLBACK_COUNT])(const mavlink_message_t *msg);
static int count = 0;
static int chan = 0;
void trecv_add_callbac(void (*f)(const mavlink_message_t *msg)) {
    assert(count < TRECV_MAX_CALLBACK_COUNT);

    int chan = mavlink_claim_channel();
    trecv_callback_arr[count++] = f;
}

void task_recv_update(void *arg) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    int size = its_i2c_link_read(buf, sizeof(buf));
    if (size < 0) {
        return;
    }
    mavlink_message_t msg;
    mavlink_status_t mst;
    for (int i = 0; i < size - 1; i++) {
        mavlink_parse_char(chan, buf[i], &msg, &mst);
    }
    if (!mavlink_parse_char(chan, buf[size - 1], &msg, &mst)) {
        return;
    }
    for (int i = 0; i < count; i++) {
        (*trecv_callback_arr[i])(&msg);
    }
}
