/* 
 * File:   handler.h
 * Author: khoai
 *
 * Created on April 21, 2015, 5:47 PM
 */

#ifndef CHANNEL_H
#define	CHANNEL_H

#include <stddef.h>
#include "common/mini-clist.h"

struct topic {
    long id;
    char *name;
    int count;
    long created_time;
    long last_active;
    struct list list;
    struct list members;
};

struct massage {
    long id;
    char *msg;
    long created_time;
    int sender;
    int topic_id;
};

struct account {
    long id;
    char *name;
    char *auth;
    long created_time;
    long last_active;
    struct list sub_lists;
    struct list pub_lists;
    struct list list;
    int fd; // fd = 0: is offline
};

int topic_register(const char *name);
int topic_unregister(const char *name);
int topic_join(const char *name, int fd);
int topic_left(const char *name, int fd);
struct topic* topic_get(const char* name);

int account_create(const char *name, const char *auth);
int account_auth(const char *name, const char *auth);
struct  account*  account_get(const char* name);

#endif	/* CHANNEL_H */

