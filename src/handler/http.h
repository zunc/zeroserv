/* 
 * File:   http.h
 * Author: khoai
 *
 * Created on August 19, 2014, 10:32 AM
 */

#ifndef HTTP_H
#define	HTTP_H

#include "type/protocols.h"
#include "type/proto_handler.h"
#include "proto_tcp.h"

int http_accept(int fd);
int http_disconnect(int fd);
int http_read(int fd);
int http_write(int fd);

static struct handler handler_http = {
    .name = "http",
    .accept = http_accept,
    .disconnect = http_disconnect,
    .read = http_read,
    .write = http_write
};

#endif	/* HTTP_H */

