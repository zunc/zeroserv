/* 
 * File:   echo_udp.h
 * Author: khoai
 *
 * Created on October 15, 2014, 2:19 PM
 */

#ifndef ECHO_UDP_H
#define	ECHO_UDP_H

#include "../type/protocols.h"
#include "../type/proto_handler.h"
#include "../proto_udp.h"

int echo_udp_init(struct listener *listener);
int echo_udp_deinit(int fd);
int echo_udp_read(int fd);
int echo_udp_write(int fd);

static struct handler handler_echo_udp = {
    .name = "echo_udp",
    .init = echo_udp_init,
    .deinit = echo_udp_deinit,
    .read = echo_udp_read,
    .write = echo_udp_write
};

#endif	/* ECHO_UDP_H */

