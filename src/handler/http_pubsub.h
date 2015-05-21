/* 
 * File:   http_pubsub.h
 * Author: khoai
 *
 * Created on May 19, 2015, 6:19 PM
 */

#ifndef HTTP_PUBSUB_H
#define	HTTP_PUBSUB_H

#include "type/protocols.h"
#include "type/proto_handler.h"
#include "proto_tcp.h"

int http_psub_request_accept();
int http_psub_accept(int fd);
int http_psub_disconnect(int fd);
int http_psub_read(int fd);
int http_psub_write(int fd);
int http_psub_load_config(void *config);

struct http_pubsub_stat {
    long conn;
    long max_conn;
};

struct session {
    char *url;
    char *cookie;
};

static struct handler handler_http_pubsub = {
    .name = "http_pubsub",
    .request_accept = http_psub_request_accept,
    .accept = http_psub_accept,
    .disconnect = http_psub_disconnect,
    .read = http_psub_read,
    .write = http_psub_write,
    .load_config = http_psub_load_config
};

#endif	/* HTTP_PUBSUB_H */

