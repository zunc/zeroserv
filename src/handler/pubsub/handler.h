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
#include "common/vector.h"

struct topic {
    long id;
    char *name;
    int count;
    long created_time;
    long last_active;
    struct list list;
    vector members;
    vector fds;
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
    vector sub_lists;
    //struct list pub_lists;
    struct list list;
    // int fd; // fd = 0: is offline
    vector fds;
};

struct topic* topic_create(const char *name);
int topic_delete(const char *name);
int topic_join(struct topic *top, struct account *acc);
int topic_sub(struct topic *top, int fd);
int topic_unsub(struct topic *top, int fd);
int topic_left(struct topic *top, struct account *acc);
struct topic* topic_get(const char *name);
struct vector topic_list();
struct vector topic_list_user(const char *name);

int account_create(const char *name, const char *auth);
int account_auth(const char *name, const char *auth);
struct account* account_get(const char* name);
struct account* account_get_by_fd(const int fd);

#endif	/* CHANNEL_H */

