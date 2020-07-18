/*
 * adc.c
 *
 *  Created on: Jul 16, 2020
 *      Author: sereshotes
 */


#include "adc.h"
#include <stdio.h>

#define ADC_PERIOD 1000 //ms

static int adc_count = 0;
uint16_t temp_int = 0;
uint16_t vref = 0;
#define tV_25   1.43f      // Напряжение (в вольтах) на датчике при температуре 25 °C.
#define tSlope  0.0043f    // Изменение напряжения (в вольтах) при изменении температуры на градус.
extern ADC_HandleTypeDef hadc1;
int is_converted = 0;


// Напряжение со встроенного термистра при 25 градусах (в милливольта)
#define INTERNAL_TEMP_V25 (1430.0f)
// Коэффициент k внутренного термистра (мВ/C
#define INTERNAL_TEMP_AVG_SLOPE (4.3f)

void adc_task_init(void *arg) {
    //HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_IT(&hadc1);
}

void adc_task_update(void *arg) {
    static uint32_t time = 0;
    if (is_converted && HAL_GetTick() - time > ADC_PERIOD) {
        time = HAL_GetTick();
        float v = temp_int * 3.3 / (float)0x0FFF;
        float t = (tV_25 - v) / tSlope + 25;


        float mv = temp_int * 3300.0f / 0x0FFF;
        float temp = (INTERNAL_TEMP_V25 - mv) / INTERNAL_TEMP_AVG_SLOPE + 25;
        printf("-----TEMP: %d\n", (int)(100 * temp));


        is_converted = 0;
        HAL_ADC_Start_IT(&hadc1);
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if(hadc->Instance == ADC1) {
        temp_int = HAL_ADC_GetValue(hadc);
    }
    is_converted = 1;
}
