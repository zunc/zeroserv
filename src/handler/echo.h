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

int echo_accept(int fd);
int echo_disconnect(int fd);
int echo_read(int fd);
int echo_write(int fd);

static struct handler handler_echo = {
	.name = "echo",
	.accept = echo_accept,
	.disconnect = echo_disconnect,
	.read = echo_read,
	.write = echo_write
};

#endif	/* HANDLER_HELLO_H */

