/* 
 * File:   handler.h
 * Author: khoai
 *
 * Created on August 18, 2014, 11:40 AM
 */

#ifndef TYPE_HANDLER_H
#define	TYPE_HANDLER_H

#include "common/mini-clist.h"
//#include "type/protocols.h"

#define PROTO_NAME_LEN 16

struct listener;

struct handler {
    char *name;
    int (*init)(struct listener* listener);
	int (*request_accept)();
    int (*accept)(int nfd);
    int (*disconnect)(int fd);
    int (*read)(int fd);
    int (*write)(int fd);
    int (*deinit)(int fd);
    int (*load_config)(void *cfg);
    void *owner;
    struct list list;
};

#endif	/* TYPE_HANDLER_H */
