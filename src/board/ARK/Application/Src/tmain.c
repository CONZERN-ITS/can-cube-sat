#include <ina219_helper.h>
#include "main.h"

#include <stdio.h>

#include "ds18b20.h"
//#include "i2c.h"
#include "its-i2c-link.h"
#include "its-time.h"
#include "mavlink/its/mavlink.h"
#include "battery.h"
#include "mavlink_system_id.h"
#include "task.h"
#include "task_ds.h"
#include "task_ina.h"
#include "task_send.h"
#include "task_recv.h"
#include "task_battery_control.h"


extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c1;
#define MSG_TEMP_DELAY 1000
#define MSG_INA_DELAY 1000

enum TEMP_STATE {
    DS_PROCESSING,
    DS_READY
} ;

#define ITS_I2CTM_HANDLER &hi2c1


void time_recv_callback(const mavlink_message_t *msg);

void task_main_init(void *arg) {
    printf("INFO: Starting task scheduling\n");
    its_i2c_link_start(ITS_I2CTM_HANDLER);
    its_time_init();
    trecv_add_callbac(time_recv_callback);

    task_t t = {0};

    t.init = task_ina_init;
    t.update = task_ina_update;
    task_create(t, 0);

    t.init = task_ds_init;
    t.update = task_ds_update;
    task_create(t, 0);

    t.init = task_send_init;
    t.update = task_send_update;
    task_create(t, 0);

    t.init = task_recv_init;
    t.update = task_recv_update;
    task_create(t, 0);

    t.init = task_battery_control_init;
    t.update = task_battery_control_update;
    task_create(t, 0);


}
void task_main_update(void *arg) {

}

int tmain(void) {
    task_t m = {0};
    m.arg = 0;
    m.init = task_main_init;
    m.update = task_main_update;
    task_create(m, 0);
    while (1) {
        task_update_all();
    }
    return 0;
}


void time_recv_callback(const mavlink_message_t *msg) {
    if (msg->msgid != MAVLINK_MSG_ID_TIMESTAMP) {
        return;
    }
    mavlink_timestamp_t mts;
    mavlink_msg_timestamp_decode(msg, &mts);

    its_time_t t;
    t.sec = mts.time_s;
    t.usec = mts.time_us / 1000;
    its_sync_time(&t);
}
