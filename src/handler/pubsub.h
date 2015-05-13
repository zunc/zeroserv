/* 
 * File:   pubsub.h
 * Author: khoai
 *
 * Created on April 21, 2015, 3:51 PM
 */

#ifndef PUBSUB_H
#define	PUBSUB_H

#include "type/protocols.h"
#include "type/proto_handler.h"
#include "proto_tcp.h"

int psub_request_accept();
int psub_accept(int fd);
int psub_disconnect(int fd);
int psub_read(int fd);
int psub_write(int fd);
int psub_load_config(void *config);

struct pubsub_stat {
    long conn;
    long max_conn;
};

static struct handler handler_pubsub = {
    .name = "pubsub",
    .request_accept = psub_request_accept,
    .accept = psub_accept,
    .disconnect = psub_disconnect,
    .read = psub_read,
    .write = psub_write,
    .load_config = psub_load_config
};

#endif	/* PUBSUB_H */

