/* 
 * File:   handler_hello.h
 * Author: khoai
 *
 * Created on August 18, 2014, 11:50 AM
 */

#ifndef ECHO_H
#define	ECHO_H

#include "../type/protocols.h"
#include "../type/proto_handler.h"
#include "../proto_tcp.h"

int echo_request_accept();
int echo_accept(int fd);
int echo_disconnect(int fd);
int echo_read(int fd);
int echo_write(int fd);
int echo_load_config(void *config);

struct echo_stat {
	long conn;
	long max_conn;
};

static struct handler handler_echo = {
    .name = "echo",
	.request_accept = echo_request_accept,
    .accept = echo_accept,
    .disconnect = echo_disconnect,
    .read = echo_read,
    .write = echo_write,
	.load_config = echo_load_config
};

#endif	/* HANDLER_HELLO_H */

