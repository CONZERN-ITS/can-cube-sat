/*
 * adc.c
 *
 *  Created on: Jul 16, 2020
 *      Author: sereshotes
 */


#include "adc.h"
#include <stdio.h>
#include "mavlink_help2.h"
#include "its-i2c-link.h"
#include "its-time.h"

#define ADC_PERIOD 1000 //ms
#define ADC_COUNT_IN_ROW 10

static int convert_count = 0;
uint16_t temp_int = 0;
uint16_t vref = 0;
#define tV_25   1.43f      // Напряжение (в вольтах) на датчике при температуре 25 °C.
#define tSlope  0.0043f    // Изменение напряжения (в вольтах) при изменении температуры на градус.
extern ADC_HandleTypeDef hadc1;
int is_converted = 0;

float t_avg = 0;

// Напряжение со встроенного термистра при 25 градусах (в милливольта)
#define INTERNAL_TEMP_V25 (1430.0f)
// Коэффициент k внутренного термистра (мВ/C
#define INTERNAL_TEMP_AVG_SLOPE (4.3f)

void adc_task_init(void *arg) {
    //HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_IT(&hadc1);
}

void adc_task_update(void *arg) {
    if (is_converted && convert_count < ADC_COUNT_IN_ROW) {
        convert_count++;

        float v = temp_int * 3.3 / (float)0x0FFF;
        float t = (tV_25 - v) / tSlope + 25;
        t_avg += t;


        //printf("-----temp: %d\n", (int)(100 * t));

        is_converted = 0;
        HAL_ADC_Start_IT(&hadc1);
    }
    static uint32_t time = 0;

    static uint8_t data[MAVLINK_MAX_PACKET_LEN] = {0};
    static uint16_t size = 0;
    static int is_sending = 0;
    if (convert_count >= ADC_COUNT_IN_ROW && HAL_GetTick() - time > ADC_PERIOD && !is_sending) {
        time = HAL_GetTick();
        convert_count = 0;
        t_avg /= ADC_COUNT_IN_ROW;

        its_time_t here = {0};
        its_gettimeofday(&here);

        mavlink_own_temp_t mot = {0};
        mot.deg = t_avg;
        mot.time_s = here.sec;
        mot.time_us = here.usec;

        mavlink_message_t msg;

        mavlink_msg_own_temp_encode(mavlink_system.sysid, mavlink_system.compid,
                &msg, &mot);


        size = mavlink_msg_to_send_buffer(data, &msg);
        is_sending = 1;
        t_avg = 0;

    }
    if (is_sending) {

        if (its_i2c_link_write(data, size) >= 0) {
            is_sending = 0;
        }
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if(hadc->Instance == ADC1) {
        temp_int = HAL_ADC_GetValue(hadc);
    }
    is_converted = 1;
}
