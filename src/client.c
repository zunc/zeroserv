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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "client.h"
#include "dbg.h"
#include "util.h"
#include "fd.h"
#include "proto_tcp.h"
#include "buffer.h"

int event_accept(int fd) {
	int s;
	struct sockaddr in_addr;
	socklen_t in_len;
	int nfd;

	in_len = sizeof in_addr;
	nfd = accept(fd, &in_addr, &in_len);

	log_info("accept: %d", nfd);
	if (nfd < 0) {
		// printf("errno=%d, EAGAIN=%d, EWOULDBLOCK=%d\n", errno, EAGAIN, EWOULDBLOCK);
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			//			log_warn("EAGAIN");
			return;
		} else {
			//			log_warn("accept");
			return;
		}
	}

	/* Make the incoming socket non-blocking and add it to the
	 * list of fds to monitor. */
	s = make_socket_non_blocking(nfd);
	if (s < 0) {
		log_warn("make_socket_non_blocking: %d", nfd);
	}

	//--- add to poller : infd
	struct listener *listen = (struct listener *) fdtab[fd].owner;
	fd_insert(nfd);

	listen->handler->accept(nfd);
	fdtab[nfd].cb[DIR_RD].f = listen->handler->read;
	fdtab[nfd].cb[DIR_WR].f = listen->handler->write;

	fdtab[nfd].state = FD_STCONN;
	fdtab[nfd].flags = FD_FL_TCP;
	fdtab[nfd].owner = fdtab[fd].owner;
	EV_FD_SET(nfd, DIR_RD);
	return 0;
}