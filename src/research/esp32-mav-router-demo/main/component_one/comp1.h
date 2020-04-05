/*
 * task.h
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#ifndef MAIN_COMPONENT_ONE_TASK_H_
#define MAIN_COMPONENT_ONE_TASK_H_

/* В этом файле модель компонента, который живет в отдельном таске
 * Для получения сообщений извне он использует очередь */


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../router/rt_endpoint_iface.h"

//! Структура - тип эндпоинта этого абонента
typedef struct its_c1_rt_endpoint_t
{
	//! Базовая структура (наследование в стиле Си)
	its_rt_endpoint_t base;
	//! Дескриптор очереди, в которую данный компонент хочет получать сообщения
	QueueHandle_t task_input_queue;
} its_c1_rt_endpoint_t;


//! Абонент маршрутизтора этого абонента
extern its_c1_rt_endpoint_t its_c1_rt_endpoint;


//! Точка входа для этого таска
void its_c1_task_entry(void * arg);


#endif /* MAIN_COMPONENT_ONE_TASK_H_ */
