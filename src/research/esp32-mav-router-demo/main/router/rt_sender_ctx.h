/*
 * rt_sender_ctx.h
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */

#ifndef MAIN_ROUTER_RT_SENDER_CTX_H_
#define MAIN_ROUTER_RT_SENDER_CTX_H_


//! Контекст отправителя сообщения
/*! Через эту стурктуру отправитель можно передать
 *  какие-нибудь метаданные получателю. Например о том, что
 *  он работает из прерывания и поэтому его нельзя задерживать и нужно использовать
 *  соответствующие функции freertos
 */
typedef struct its_rt_sender_ctx_t
{
	//! from_isr != 0 => работа из прерывания
	int from_isr;
} its_rt_sender_ctx_t;


#endif /* MAIN_ROUTER_RT_SENDER_CTX_H_ */
