/*
 * bms.h
 *
 *  Created on: 16 мар. 2020 г.
 *      Author: sereshotes
 */

#ifndef INC_BMS_H_
#define INC_BMS_H_


#include "main.h"

struct bms_t {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
};

void bms_init(struct bms_t *hbms, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void bms_off(struct bms_t *hbms);
void bms_on(struct bms_t *hbms);

#endif /* INC_BMS_H_ */
