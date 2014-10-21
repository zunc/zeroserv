/* 
 * File:   proto_udp.h
 * Author: khoai
 *
 * Created on October 15, 2014, 1:50 PM
 */

#ifndef PROTO_UDP_H
#define	PROTO_UDP_H

#include "type/protocols.h"

int udp_recv(int fd);
int udp_write(int fd);

int udp_bind_listeners(struct protocol *proto);
int udp_unbind(struct protocol *proto);
int udp_enable(struct protocol *proto);
//void tcp_add_listener(struct protocol *proto, struct listener *listener);
int udp_create_listener(char *name, int port, struct handler *handler);

static struct protocol proto_udpv4 = {
	.name = "udp_socket",
	.sock_domain = AF_INET,
	.sock_type = SOCK_DGRAM,
	.sock_prot = IPPROTO_UDP,
	.sock_family = AF_INET,
	.sock_addrlen = sizeof (struct sockaddr_in),
	.l3_addrlen = 32 / 8,
	.read = &udp_recv,
	.write = &udp_write,
	.bind_all = udp_bind_listeners,
	.unbind_all = udp_unbind,
	.enable_all = udp_enable,
	.listeners = LIST_HEAD_INIT(proto_udpv4.listeners),
	.nb_listeners = 0,
};

#endif	/* PROTO_UDP_H */

