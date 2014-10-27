#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "echo.h"
#include "../buffer.h"
#include "../protos.h"
#include "../dbg.h"
#include "../common/mini-clist.h"
#include "../fd.h"
#include "../common.h"
#include "../handler.h"
#include "../common/ticks.h"
#include "../common/time.h"
#include "../task.h"

#define HTTP_BUFFER_SIZE 4096
#define RUN_ONCE	0
#define MAINTAIN_TIME	10000
uint32_t _conn;

static int close_fd(int fd);

struct task * maintain_stat(struct task *t) {
	log_info("stat_conn: %d", _conn);
	t->expire = tick_add(now_ms, MAINTAIN_TIME);
	return t;
}

struct task * timeout(struct task *t) {
	log_info("timeout");
	int fd = (int) t->context;
	free(t);
	echo_disconnect(fd);
	return NULL;
}

int echo_accept(int fd) {
	struct buffer *b = (struct buffer*) malloc(sizeof (struct buffer));
	buffer_init(b, HTTP_BUFFER_SIZE);
	fdtab[fd].cb[DIR_RD].b = fdtab[fd].cb[DIR_WR].b = b;
	printf("[%*d] connected\n", 4, fd);

	// setup a task: drop connection in 5s
	struct task *t = NULL;
	if ((t = task_new()) == NULL)
		return -1;

	t->context = (void*) fd;
	t->expire = tick_add(now_ms, S_TO_TICKS(20));
	t->process = timeout;
	task_queue(t);
	fdtab[fd].context = t;
	_conn++;
	return 0;
}

int echo_disconnect(int fd) {
	log_info("[%*d] disconnect\n", 4, fd);
	struct buffer *b = fdtab[fd].cb[DIR_WR].b;
	buffer_free(b);
	fdtab[fd].cb[DIR_WR].b = NULL;
	_conn--;
	return 0;
}

static int close_fd(int fd) {
	echo_disconnect(fd);
	fd_delete(fd);
	struct task* t = fdtab[fd].context;
	if (t) {
		task_delete(t);
		task_free(t);
		fdtab[fd].context = NULL;
	}
}

int echo_read(int fd) {
	struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
	int remain = buffer_remain_write(ib);
	int n = recv(fd, ib->r, remain, 0);
	if (n > 0) {
		ib->r += n;
		log_dbg("recv: %d", n);
		EV_FD_SET(fd, DIR_WR);
		*ib->r = 0;
		printf("[%*d] %s", 4, fd, ib->curr);
	} else {
		log_dbg("recv fail: %d", n);
		printf("[%*d]disconnect\n", 4, fd);
		close_fd(fd);
	}
	return 0;
}

int echo_write(int fd) {
	log_dbg("echo_write");
	struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
	int remain = buffer_remain_read(ob);
	int n = send(fd, ob->curr, remain, 0);
	if (n > 0) {
		ob->curr += n;
		if (buffer_empty(ob)) {
			buffer_reset(ob);
			EV_FD_CLR(fd, DIR_WR);
		}
	} else {
		log_info("send: (%d)", n);
		close_fd(fd);
		echo_disconnect(fd);
	}
#if RUN_ONCE
	fd_delete(fd);
	echo_disconnect(fd);
#endif
	return 0;
}

__attribute__((constructor))
static void __echo_handler_init(void) {
	_conn = 0;
//	// maintain schedule
//	struct task* t = NULL;
//	if ((t = task_new()) == NULL) {
//		log_fatal("task_new fail");
//	}
//	t->process = maintain_stat;
//	t->context = NULL;
//	t->expire = TICK_ETERNITY;
//	task_schedule(t, tick_add(now_ms, MAINTAIN_TIME));

	handler_register(&handler_echo);
}