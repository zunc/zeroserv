#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "http.h"
#include "../buffer.h"
#include "../protos.h"
#include "../dbg.h"
#include "../common/mini-clist.h"
#include "../fd.h"
#include "../common.h"
#include "../common/compiler.h"

//--- string proc
#include <string.h>

#define SPLITTER " "

#define LOG_REQ 0
#define HTTP_LOG(format, ...) if (LOG_REQ) log(format, "HTTP", ## __VA_ARGS__);
#define HTTP_WARN(format, ...) if (LOG_REQ) log_warn(format, ## __VA_ARGS__);

#define HTTP_OK		200
#define HTTP_404	404

#define METHOD_GET	"GET"
#define METHOD_POST "POST"

#define CHK_METHOD(method, METHOD) (!strncmp(method, METHOD, sizeof(METHOD)))

#define MSG_HELLO "HTTP/1.1 200 OK\r\n" \
			"Server: zeroxy\r\n" \
			"Content-Type: text/html; charset=UTF-8\r\n" \
			"Content-Length: 5\r\n" \
			"Connection: close\r\n\r\n" \
			"hello"

static char msg_hello[] = MSG_HELLO;

static int http_response(int fd, int err_code, char *rep) {
	struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
	sprintf(ob->r,
			"HTTP/1.1 %d OK\r\n"
			"Server: zeroxy\r\n"
			"Content-Type: text/html; charset=UTF-8\r\n"
			"Content-Length: %ld\r\n"
			"Connection: close\r\n\r\n"
			"%s",
			err_code, strlen(rep), rep);
	ob->r += strlen(ob->r);

	//	strcpy(ob->w, msg_hello);
	//	ob->w += sizeof(msg_hello)-1;
	return strlen(ob->buff);
}

static int http_parse(int fd, char *req, int len) {
	char *first = strstr(req, SPLITTER);
	if (!first) goto exit;

	// check method
	char method[12];
	int len_method = first - req;
	if (len_method > sizeof (method)) goto exit;
	memcpy(method, req, len_method);
	method[len_method] = 0;
	if CHK_METHOD(method, METHOD_GET) {
		//		printf("GET\n");
	} else if CHK_METHOD(method, METHOD_POST) {
		//		printf("POST\n");
	} else {
		HTTP_WARN("spoil method");
		goto exit;
	}

	char *last = strstr(++first, SPLITTER);
	if (!last) goto exit;

	int len_url = (int) (last - first);
	//	char *url = first;
	//	url[len_url] = 0;
	char url[128]; //<!> optimal it
	if (likely(len_url < (sizeof (url) - 1))) {
		memcpy(url, first, len_url);
		url[len_url] = 0;
		if (!strcmp(url, "/stat")) {
			//stat
			http_response(fd, HTTP_OK, "stat_statistic");
		} else {
			http_response(fd, HTTP_OK, url);
		}
	} else {
		http_response(fd, HTTP_OK, "too long url");
	}
	//	printf("url: %s\n", req_url);
	return 1;
exit:
	return 0;
}

int http_accept(int fd) {
	struct buffer *ib = (struct buffer*) malloc(sizeof (struct buffer));
	struct buffer *ob = (struct buffer*) malloc(sizeof (struct buffer));
	fdtab[fd].cb[DIR_RD].b = ib;
	fdtab[fd].cb[DIR_WR].b = ob;
	buffer_reset(ib);
	buffer_reset(ob);
	return 0;
}

int http_disconnect(int fd) {
	FREE(fdtab[fd].cb[DIR_RD].b);
	FREE(fdtab[fd].cb[DIR_WR].b);
	return 0;
}

int http_read(int fd) {
	//	log_info("[-] http_read");
	struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
	int remain = buffer_remain_write(ib);
	int n = recv(fd, ib->r, remain, 0);
	if (likely(n > 0)) {
		ib->r += n;
		log_dbg("recv: %d", n);
		if (http_parse(fd, ib->buff, n)) {
			//			log_info("parse DONE");
			EV_FD_SET(fd, DIR_WR);
			EV_FD_CLR(fd, DIR_RD);
		}
	} else {
		log_dbg("recv fail: %d", n);
		EV_FD_CLR(fd, DIR_RD);
	}
	return 0;
}

int http_write(int fd) {
	log_dbg("[-] http_write");
	struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
	int remain = buffer_remain_read(ob);
	int n = send(fd, ob->curr, remain, 0);
	if (likely(n > 0)) {
		ob->curr += n;
	} else {
		fd_delete(fd);
		http_disconnect(fd);
		return -1;
	}

	if (unlikely(!buffer_remain_read(ob))) {
		fd_delete(fd);
		http_disconnect(fd);
		return 0;
	}
	return 0;
}

__attribute__((constructor))
static void __echo_handler_init(void) {
	handler_register(&handler_http);
}