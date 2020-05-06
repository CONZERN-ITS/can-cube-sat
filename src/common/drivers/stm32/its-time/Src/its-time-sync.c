#include "main.h"
#include "its-time.h"
#include <stdio.h>


static its_time_t last_exti;
static volatile int is_intertupted = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
    case TIME_Pin: {
        is_intertupted = 1;
        its_gettimeofday(&last_exti);
    }
    default:
        break;
    }
}


void its_sync_time(its_time_t *from_bcs) {
    if (!is_intertupted) {
        return;
    };
    is_intertupted = 0;


    its_time_t now;
    its_gettimeofday(&now);
    uint64_t new = (now.sec * 1ll - last_exti.sec + from_bcs->sec) * 1000 + now.usec - last_exti.usec + from_bcs->usec;
    now.sec = new / 1000;
    now.usec = new % 1000;
    its_settimeofday(&now);
}
