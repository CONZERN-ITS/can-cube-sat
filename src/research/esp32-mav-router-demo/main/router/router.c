/*
 * router.c
 *
 *  Created on: 5 апр. 2020 г.
 *      Author: snork
 */


#include "router.h"

#include "../component_one/comp1.h"
#include "../component_two/comp2.h"

static its_rt_endpoint_t * const system_endpoints[] = {
		&its_c1_rt_endpoint.base,
		&its_c2_rt_endpoint.base
};


void its_rt_route(const its_rt_sender_ctx_t * sender_ctx, const mavlink_message_t * msg)
{
	// Просто проходим по всем эндпоинтам и каждого кормим этим пакетом
	for (size_t i = 0; i < sizeof(system_endpoints)/sizeof(*system_endpoints); i++)
	{
		its_rt_endpoint_t * const ep = system_endpoints[i];
		ep->acceptor(ep, sender_ctx, msg);
	}
}
