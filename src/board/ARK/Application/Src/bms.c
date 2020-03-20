/*
 * bms.c
 *
 *  Created on: 18 мар. 2020 г.
 *      Author: sereshotes
 */


#include "bms.h"


void bms_init(struct bms_t *hbms, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    hbms->GPIO_Pin = GPIO_Pin;
    hbms->GPIOx = GPIOx;
}
void bms_off(struct bms_t *hbms) {
    ((hbms->GPIOx)->BSRR = (uint32_t)(((uint32_t)hbms->GPIO_Pin) << 16));
}
void bms_on(struct bms_t *hbms) {
    ((hbms->GPIOx)->BSRR = (uint32_t)(hbms->GPIO_Pin));
}
