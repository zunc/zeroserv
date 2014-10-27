/* 
 * File:   routing.h
 * Author: khoai
 *
 * Created on October 16, 2014, 11:05 AM
 */

#ifndef TROUTING_H
#define	TROUTING_H

#include "../type/protocols.h"
#include "../type/proto_handler.h"
#include "../proto_tcp.h"
//#include "../handler_udp/routing.h"

int trouting_init(struct listener *listener);
int trouting_deinit(int fd);
int trouting_accept(int fd);
int trouting_read(int fd);
int trouting_write(int fd);
int trouting_disconnect(int fd);
int trouting_load_config(void *config);

static struct handler handler_trouting = {
	.name = "trouter",
	.init = trouting_init,
	.deinit = trouting_deinit,
	.accept = trouting_accept,
	.read = trouting_read,
	.write = trouting_write,
	.load_config = trouting_load_config
};

struct router_setting {
	int retry;
	int timeout;
	int max_packet;
	char *layer;
};

struct router_stat {
	uint64_t recv;
	uint64_t broken;
	uint64_t retries;
	uint64_t drop;
};

extern struct router_setting trouter_setting;
extern struct router_stat trouter_stat;

#endif	/* ROUTING_H */

