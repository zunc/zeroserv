#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <libconfig.h>

#include "echo.h"
#include "buffer.h"
#include "protos.h"
#include "dbg.h"
#include "fd.h"
#include "common/mini-clist.h"
#include "common.h"
#include "handler.h"
#include "common/ticks.h"
#include "common/time.h"
#include "task.h"

#define HTTP_BUFFER_SIZE 4096
#define RUN_ONCE	0
#define MAINTAIN_TIME	10000

struct echo_stat _stat;

struct task * maintain_stat(struct task *t) {
	log_info("stat_conn: %ld", _stat.conn);
	t->expire = tick_add(now_ms, MAINTAIN_TIME);
	return t;
}

struct task * timeout(struct task *t) {
	log_info("timeout");
	//	int fd = (int) t->context;
	//	free(t);
	//	echo_disconnect(fd);
	return NULL;
}

int echo_request_accept() {
	if (_stat.conn >= _stat.max_conn) {
		log_info("[echo] connection reach max(%ld)", _stat.max_conn);
		return ZB_FAIL;
	}
	return ZB_NOP;
}

int echo_accept(int nfd) {
	struct buffer *b = (struct buffer*) malloc(sizeof (struct buffer));
	buffer_init(b, HTTP_BUFFER_SIZE);
	fdtab[nfd].cb[DIR_RD].b = fdtab[nfd].cb[DIR_WR].b = b;
	printf("[%*d] connected\n", 4, nfd);
	if (_stat.conn >= _stat.max_conn) {
		log_info("[echo] connection reach max(%ld)", _stat.max_conn);
		return ZB_FAIL;
	}
	_stat.conn++;
	return ZB_NOP;
}

int echo_disconnect(int fd) {
	log_info("[%*d] disconnect", 4, fd);
	struct buffer *b = fdtab[fd].cb[DIR_WR].b;
	buffer_free(b);
	fdtab[fd].cb[DIR_WR].b = NULL;
	_stat.conn--;
	return ZB_NOP;
}

int echo_read(int fd) {
	struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
	int remain = buffer_remain_write(ib);
	int n = recv(fd, ib->r, remain, 0);
	int ret = ZB_NOP;
	if (n > 0) {
		ib->r += n;
		log_dbg("recv: %d", n);
		ret |= ZB_SET_WR;
		*ib->r = 0;
		printf("[%*d] %s", 4, fd, ib->curr);
	} else {
		log_dbg("recv fail: %d", n);
		printf("[%*d]disconnect\n", 4, fd);
		return ZB_CLOSE;
	}
	return ret;
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
			return ZB_CLOSE_WR;
		}
	} else {
		log_info("send: (%d)", n);
		return ZB_CLOSE;
	}
#if RUN_ONCE
	return ZB_CLOSE;
#endif
	return ZB_NOP;
}

int echo_load_config(void *config) {
	const config_setting_t *echo_cfg = (const config_setting_t*) config;
	// get routing
	if (!config_setting_lookup_int64(echo_cfg, "max_conn", (long long *) &_stat.max_conn)) {
		log_fatal("handler(%s) incorrect config", handler_echo.name);
	} else {
		//log_info("%s{max_conn(%ld)}", handler_echo.name, _stat.max_conn);
	}
	return 0;
}

__attribute__((constructor))
static void __echo_handler_init(void) {
	_stat.conn = 0;
	handler_register(&handler_echo);
}