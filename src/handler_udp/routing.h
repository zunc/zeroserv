/* 
 * File:   routing.h
 * Author: khoai
 *
 * Created on October 16, 2014, 11:05 AM
 */

#ifndef ROUTING_H
#define	ROUTING_H

#include "../type/protocols.h"
#include "../type/proto_handler.h"
#include "../proto_udp.h"

int routing_init(struct listener *listener);
int routing_deinit(int fd);
int routing_read(int fd);
int routing_write(int fd);
int routing_load_config(void *config);

static struct handler handler_routing = {
	.name = "router",
	.init = routing_init,
	.deinit = routing_deinit,
	.read = routing_read,
	.write = routing_write,
	.load_config = routing_load_config
};

struct router_setting {
	int retry;
	int timeout;
	int max_packet;
};

struct router_stat {
	uint64_t recv;
	uint64_t broken;
	uint64_t retries;
	uint64_t drop;
};

extern struct router_setting router_setting;
extern struct router_stat router_stat;

#endif	/* ROUTING_H */

