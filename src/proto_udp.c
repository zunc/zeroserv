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
#include "proto_udp.h"
#include "fd.h"
#include "poller.h"
#include "client.h"

int udp_recv(int fd) {
	struct protocol *proto = fdtab[fd].owner;
	log_info("udp_recv: %d", fd);
	return 0;
}

int udp_write(int fd) {
	struct protocol *proto = fdtab[fd].owner;
	log_info("udp_write: %d", fd);
	return 0;
}

void udp_add_listener(struct listener *listener) {
	listener->proto = &proto_udpv4;
	LIST_ADDQ(&proto_udpv4.listeners, &listener->proto_list);
	proto_udpv4.nb_listeners++;
}

int udp_create_listener(char *name, int port, struct handler *handler) {
	struct listener *listen = (struct listener *) malloc(sizeof (struct listener));
	listen->name = name;
	listen->port = port;
	listen->handler = handler;
	udp_add_listener(listen);
	return 0;
}

int udp_create_and_bind(uint16_t port) {
	int s;
	struct sockaddr_in serv_addr;

	s = socket(proto_udpv4.sock_domain, proto_udpv4.sock_type, proto_udpv4.sock_prot);
	if (s < 0) {
		perror("ERROR opening socket");
		log_fatal("udp_create_and_bind");
	}

	bzero((char *) &serv_addr, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	int yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	if (bind(s, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
		perror("ERROR");
		log_fatal("ERROR on binding: %d", port);
	}
	return s;
}

int udp_bind(struct listener *listener) {
	ASSERT(listener);
	int r;
	int fd = udp_create_and_bind(listener->port);
	if (fd < 0) {
		log_fatal("[%s] udp_create_and_bind fail", proto_udpv4.name);
		return fd;
	}

	if (make_socket_non_blocking(fd) < 0) {
		log_fatal("[%s] make_socket_non_blocking fail", proto_udpv4.name);
		//udp_unbind(listener);
		return -1;
	}

	fd_insert(fd);
	
	fdtab[fd].cb[DIR_RD].f = listener->handler->read;
	fdtab[fd].cb[DIR_WR].f = listener->handler->write;
	fdtab[fd].state = FD_STLISTEN;
	fdtab[fd].flags = FD_FL_TCP;
	fdtab[fd].owner = listener;
	listener->fd = fd;
	
	// in udp, use read/write callback direct to handler instance event_accept
	// so init buffer is listen
	if (listener->handler->init)
		listener->handler->init(listener);
	
	log_dbg("listen: %d", listener->port);
	return 0;
}

int udp_bind_listeners(struct protocol *proto) {
	struct listener *listener;
	int err = 0;

	list_for_each_entry(listener, &proto->listeners, proto_list) {
		log_info("[%s] -> %d", listener->name, listener->port);
		err |= udp_bind(listener);
		if (err < 0) {
			break;
		}
	}
	return err;
}

int udp_unbind(struct protocol *proto) {
	struct listener *listener;
	int err = 0;

	list_for_each_entry(listener, &proto->listeners, proto_list) {
		log_info("[%s] -> %d", listener->name, listener->port);
		if (listener->fd) {
			log_info("udp_unbind: %d", listener->fd);
			err |= close(listener->fd);
		}
		if (err < 0) {
			break;
		}
	}
	return err;
}

int udp_enable(struct protocol *proto) {
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
static void __udp_protocol_init(void) {
	protocol_register(&proto_udpv4);
}