/*
 *
 * Copyright 2000-2009 Willy Tarreau <w@1wt.eu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "dbg.h"
#include "common.h"
#include "util.h"
#include "proto_tcp.h"
#include "fd.h"
#include "poller.h"
#include "client.h"

int tcp_recv(int fd) {
	struct protocol *proto = fdtab[fd].owner;
	log_info("tcp_recv: %d", fd);
	return 0;
}

int tcp_write(int fd) {
	struct protocol *proto = fdtab[fd].owner;
	log_info("tcp_write: %d", fd);
	return 0;
}

void tcp_add_listener(struct listener *listener) {
	listener->proto = &proto_tcpv4;
	LIST_ADDQ(&proto_tcpv4.listeners, &listener->proto_list);
	proto_tcpv4.nb_listeners++;
}

int tcp_create_listener(char *name, int port, struct handler *handler) {
	struct listener *listen = (struct listener *) malloc(sizeof (struct listener));
	listen->name = name;
	listen->port = port;
	listen->handler = handler;
	tcp_add_listener(listen);
	return 0;
}

int tcp_create_and_bind(uint16_t port) {
	int s;
	struct sockaddr_in serv_addr;

	s = socket(proto_tcpv4.sock_domain, proto_tcpv4.sock_type, proto_tcpv4.sock_prot);
	if (s < 0) {
		log_fatal("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	int yes = 1;
#if USE_REUSEADDR
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
#endif

#if USE_REUSEPORT
	yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof (yes)) == -1) {
		perror("setsockopt");
		exit(1);
	}
#endif

	if (bind(s, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
		perror("ERROR");
		log_fatal("ERROR on binding: %d", port);
	}
	return s;
}

int tcp_bind(struct listener *listener) {
	ASSERT(listener);
	int r;
	int fd = tcp_create_and_bind(listener->port);
	if (fd < 0) {
		log_fatal("[%s] tcp_create_and_bind fail", proto_tcpv4.name);
		return fd;
	}

	if (make_socket_non_blocking(fd) < 0) {
		log_fatal("[%s] make_socket_non_blocking fail", proto_tcpv4.name);
		//		tcp_unbind(listener);
		return -1;
	}

	r = listen(fd, SOMAXCONN);
	if (r < 0) {
		perror("listen");
		log_warn("listen(%d) fail", listener->port);
	}

	// set poller event_accept
	if (listener->handler->init)
		listener->handler->init(listener);

	fd_insert(fd);
	//	poll_set_listen(fd);
	fdtab[fd].cb[DIR_RD].f = event_accept;
	fdtab[fd].cb[DIR_WR].f = NULL;
	fdtab[fd].state = FD_STLISTEN;
	fdtab[fd].flags = FD_FL_TCP;
	fdtab[fd].owner = listener;
	listener->fd = fd;

	log_dbg("listen: %d", listener->port);
	return 0;
}

int tcp_bind_listeners(struct protocol *proto) {
	struct listener *listener;
	int err = 0;

	list_for_each_entry(listener, &proto->listeners, proto_list) {
		log_info("[%s] -> %d", listener->name, listener->port);
		err |= tcp_bind(listener);
		if (err < 0) {
			break;
		}
	}
	return err;
}

int tcp_unbind(struct protocol *proto) {
	struct listener *listener;
	int err = 0;

	list_for_each_entry(listener, &proto->listeners, proto_list) {
		log_info("[%s] -> %d", listener->name, listener->port);
		if (listener->fd) {
			log_info("tcp_unbind: %d", listener->fd);
			err |= close(listener->fd);
		}
		if (err < 0) {
			break;
		}
	}
	return err;
}

int tcp_enable(struct protocol *proto) {
	struct listener *listener;
	int err = 0;

	list_for_each_entry(listener, &proto->listeners, proto_list) {
		EV_FD_SET(listener->fd, DIR_RD);
		if (err < 0) {
			break;
		}
	}
	return 0;
}

__attribute__((constructor))
static void __tcp_protocol_init(void) {
	protocol_register(&proto_tcpv4);
}