/*
 * comp2.h
 *
 *  Created on: 6 апр. 2020 г.
 *      Author: snork
 */

#ifndef MAIN_COMPONENT_TWO_COMP2_H_
#define MAIN_COMPONENT_TWO_COMP2_H_


/* В этом файле модель компонента, который не имеет собственного таска
 * а, имеет лишь собственные ресурсы. Поэтому получаемые сообщения он разибрает не отходя от кассы */


#include "../router/rt_endpoint_iface.h"


//! Структура - тип эндпоинта этого абонента
typedef struct its_c2_rt_endpoint_t
{
	//! Базовая структура (наследование в стиле Си)
	its_rt_endpoint_t base;

	// А больше ничего тут и нет
} its_c2_rt_endpoint_t;


//! Абонент маршрутизатора этого компонента
extern its_c2_rt_endpoint_t its_c2_rt_endpoint;

//! Инициализация компонента
void its_c2_init(void);

#endif /* MAIN_COMPONENT_TWO_COMP2_H_ */
