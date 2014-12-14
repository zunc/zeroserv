/* 
 * File:   proto_raw.h
 * Author: khoai
 *
 * Created on August 15, 2014, 2:06 PM
 */

#ifndef PROTO_RAW_H
#define	PROTO_RAW_H

#include "type/protocols.h"

int tcp_recv(int fd);
int tcp_write(int fd);

int tcp_bind_listeners(struct protocol *proto);
int tcp_unbind(struct protocol *proto);
int tcp_enable(struct protocol *proto);
//void tcp_add_listener(struct protocol *proto, struct listener *listener);
int tcp_create_listener(char *name, int port, struct handler *handler);

static struct protocol proto_tcpv4 = {
    .name = "tcp_socket",
    .sock_domain = AF_INET,
    .sock_type = SOCK_STREAM,
    .sock_prot = IPPROTO_TCP,
    .sock_family = AF_INET,
    .sock_addrlen = sizeof (struct sockaddr_in),
    .l3_addrlen = 32 / 8,
    .read = &tcp_recv,
    .write = &tcp_write,
    .bind_all = tcp_bind_listeners,
    .unbind_all = tcp_unbind,
    .enable_all = tcp_enable,
    .listeners = LIST_HEAD_INIT(proto_tcpv4.listeners),
    .nb_listeners = 0,
};

#endif	/* PROTO_RAW_H */

