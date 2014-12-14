/* 
 * File:   memcached.h
 * Author: khoai
 *
 * Created on September 9, 2014, 9:05 PM
 */

/*
 *	a memcached clone 
 */

#ifndef MEMCACHED_H
#define	MEMCACHED_H

#include "../type/protocols.h"
#include "../type/proto_handler.h"
#include "../proto_tcp.h"

int mc_init(struct listener *listener);
int mc_accept(int fd);
int mc_disconnect(int fd);
int mc_read(int fd);
int mc_write(int fd);

static struct handler handler_memcached = {
    .name = "memcached",
    .init = mc_init,
    .accept = mc_accept,
    .disconnect = mc_disconnect,
    .read = mc_read,
    .write = mc_write
};

extern struct stats stats;

#endif	/* MEMCACHED_H */

