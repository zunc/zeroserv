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

#include <stdio.h>
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

#include "poller.h"
#include "fd.h"
#include "dbg.h"
#include "global.h"
#include "common.h"
#include "common/compiler.h"

static struct epoll_event *epoll_events;
static int epoll_fd;
static struct epoll_event ev;

/* This is what we store in a list. It consists in old values and fds to detect changes. */
struct fd_chg {
	unsigned int prev : 2; // previous state mask. New one is in fd_evts.
	unsigned int fd : 30; // file descriptor
};

static int nbchanges = 0; // number of changes pending
static struct fd_chg *chg_list = NULL; // list of changes
static struct fd_chg **chg_ptr = NULL; // per-fd changes

/* Each 32-bit word contains 2-bit descriptors of the latest state for 16 FDs :
 *   desc = (u32 >> (2*fd)) & 3
 *   desc = 0 : FD not set
 *          1 : WRITE not set, READ set
 *          2 : WRITE set, READ not set
 *          3 : WRITE set, READ set
 */

static uint32_t *fd_evts;

/* converts a direction to a single bitmask.
 *  0 => 1
 *  1 => 2
 */
#define DIR2MSK(dir) ((dir) + 1)

/* converts an FD to an fd_evts offset and to a bit shift */
#define FD2OFS(fd)   ((uint32_t)(fd) >> 4)
#define FD2BIT(fd)   (((uint32_t)(fd) & 0xF) << 1)
#define FD2MSK(fd)   (3 << FD2BIT(fd))

/*
 * Returns non-zero if direction <dir> is already set for <fd>.
 */
int __fd_is_set(const int fd, int dir) {
	return (fd_evts[FD2OFS(fd)] >> FD2BIT(fd)) & DIR2MSK(dir);
}

/*
 * Adds, mods or deletes <fd> according to current status and to new desired
 * mask <dmask> :
 *
 *    0 = nothing
 *    1 = EPOLLIN
 *    2 = EPOLLOUT
 *    3 = EPOLLIN | EPOLLOUT
 *
 */
static int dmsk2event[4] = {0, EPOLLIN, EPOLLOUT, EPOLLIN | EPOLLOUT};

void fd_flush_changes() {
	uint32_t ofs;
	int opcode;
	int prev, next;
	int chg, fd;

	for (chg = 0; chg < nbchanges; chg++) {
		prev = chg_list[chg].prev;
		fd = chg_list[chg].fd;
		chg_ptr[fd] = NULL;

		ofs = FD2OFS(fd);
		next = (fd_evts[ofs] >> FD2BIT(fd)) & 3;

		if (prev == next)
			/* if the value was unchanged, do nothing */
			continue;

		ev.events = dmsk2event[next];
		ev.data.fd = fd;

		if (prev) {
			if (!next) {
				log_dbg("EPOLL_CTL_DEL");
				/* we want to delete it now */
				opcode = EPOLL_CTL_DEL;
			} else {
				log_dbg("EPOLL_CTL_MOD");
				/* we want to switch it */
				opcode = EPOLL_CTL_MOD;
			}
		} else {
			log_dbg("EPOLL_CTL_ADD");
			/* the FD did not exist, let's add it */
			opcode = EPOLL_CTL_ADD;
		}

		epoll_ctl(epoll_fd, opcode, fd, &ev);
	}
	nbchanges = 0;
}

void alloc_chg_list(const int fd, int old_evt) {
	struct fd_chg *ptr;
	if (unlikely(chg_ptr[fd] != NULL))
		return;

#if LIMIT_NUMBER_OF_CHANGES
	if (nbchanges > 2)
		fd_flush_changes();
#endif
	//	log_info("change: %d", fd);
	ptr = &chg_list[nbchanges++];
	chg_ptr[fd] = ptr;
	ptr->fd = fd;
	ptr->prev = old_evt;
}

int __fd_set(const int fd, int dir) {
	uint32_t ofs = FD2OFS(fd);
	uint32_t dmsk = DIR2MSK(dir);
	uint32_t old_evt;

	old_evt = fd_evts[ofs] >> FD2BIT(fd);
	old_evt &= 3;
	if (unlikely(old_evt & dmsk))
		return 0;

	alloc_chg_list(fd, old_evt);
	dmsk <<= FD2BIT(fd);
	fd_evts[ofs] |= dmsk;
	return 1;
}

int __fd_clr(const int fd, int dir) {
	uint32_t ofs = FD2OFS(fd);
	uint32_t dmsk = DIR2MSK(dir);
	uint32_t old_evt;

	old_evt = fd_evts[ofs] >> FD2BIT(fd);
	old_evt &= 3;
	if (unlikely(!(old_evt & dmsk)))
		return 0;

	alloc_chg_list(fd, old_evt);
	dmsk <<= FD2BIT(fd);
	fd_evts[ofs] &= ~dmsk;
	return 1;
}

void __fd_rem(int fd) {
	uint32_t ofs = FD2OFS(fd);
	uint32_t old_evt;

	old_evt = fd_evts[ofs] >> FD2BIT(fd);
	old_evt &= 3;

	if (unlikely(!old_evt))
		return;

	alloc_chg_list(fd, old_evt);
	fd_evts[ofs] &= ~FD2MSK(fd);
	return;
}

/*
 * On valid epoll() implementations, a call to close() automatically removes
 * the fds. This means that the FD will appear as previously unset.
 */
void __fd_clo(int fd) {
	struct fd_chg *ptr;
	fd_evts[FD2OFS(fd)] &= ~FD2MSK(fd);
	ptr = chg_ptr[fd];
	if (ptr) {
		ptr->prev = 0;
		chg_ptr[fd] = NULL;
	}
	return;
}

int poll_init() {
	epoll_fd = epoll_create(global.maxsock + 1);
	if (epoll_fd == -1) {
		log_fatal("epoll_create");
	}
	epoll_events = calloc(1, global.maxsock * sizeof (struct epoll_event));
	int fd_set_bytes = 4 * (global.maxsock + 15) / 16;
	fd_evts = (uint32_t *) calloc(1, fd_set_bytes);
	chg_list = (struct fd_chg *) calloc(1, sizeof (struct fd_chg) * global.maxsock);
	chg_ptr = (struct fd_chg **) calloc(1, sizeof (struct fd_chg*) * global.maxsock);
	return 0;
}

int poll_term() {
	log_info("close: %d", epoll_fd);
	close(epoll_fd);
	FREE(epoll_events);
	FREE(fd_evts);
	FREE(chg_list);
	FREE(chg_ptr);
	return 0;
}

#define MAX_DELAY_MS 1000

int poll_do(int exp) {
	int n, i, fd;
	int wait_time;

	if (nbchanges)
		fd_flush_changes();

	if (exp > MAX_DELAY_MS)
		wait_time = MAX_DELAY_MS;
	else
		wait_time = exp;
	//	<!> impl it
	//	else if (tick_is_expired(exp, now_ms))
	//		wait_time = 0;

	n = epoll_wait(epoll_fd, epoll_events, 200, wait_time);
	tv_update_date(wait_time, n);
	for (i = 0; i < n; i++) {
		int event = epoll_events[i].events;
		fd = epoll_events[i].data.fd;

		if (fdtab[fd].state == FD_STCLOSE)
			continue;

		// check DIR_RD mask
		if (fdtab[fd].state == FD_STCLOSE)
			continue;
		
		if (event & (EPOLLIN | EPOLLERR | EPOLLHUP))
			fdtab[fd].cb[DIR_RD].f(fd);

		// check DIR_WR mask
		if (fdtab[fd].state == FD_STCLOSE)
			continue;
		if (event & (EPOLLOUT | EPOLLERR | EPOLLHUP))
			fdtab[fd].cb[DIR_WR].f(fd);
	}
	return 0;
}

int poll_fork() {
	if (epoll_fd >= 0)
		close(epoll_fd);
	epoll_fd = epoll_create(global.maxsock + 1);
	if (epoll_fd < 0)
		return 0;
	return 1;
}