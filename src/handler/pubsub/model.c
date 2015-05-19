#include "model.h"
#include "type/fd.h"
#include "handler.h"
#include "buffer.h"
#include "fd.h"
#include "idzen.h"
#include <stdio.h>

int model_acc_create(const int fd, const char* user, const char* auth) {
    printf("[CREATE] user(\"%s\"), pass(\"%s\")\n", user, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (account_create(user, auth) < 0) {
        buffer_sprintf(ob, "> create user error\n");
    } else {
        buffer_sprintf(ob, "> create user(\"%s\") success\n", user);
    }
    return ZB_SET_WR;
}

int model_acc_auth(const int fd, const char* user, const char* auth) {
    printf("[LOGIN] user(\"%s\"), pass(\"%s\")\n", user, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct account* acc = account_get(user);
    if (acc && !strncmp(acc->auth, auth, strlen(acc->auth))) {
        buffer_sprintf(ob, "> auth user(\"%s\") success\n", user);
        acc->fd = fd;
    } else {
        buffer_sprintf(ob, "> auth user error\n");
    }
    return ZB_SET_WR;
}

int model_acc_offline(const int fd) {
    struct account* acc = account_get_by_fd(fd);
    if (acc) {
        printf("[OFFLINE] fd(%d), user(%s)\n", fd, acc->name);
        acc->fd = 0;
    }
    return !acc;
}

int model_sub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_join(top, acc)) {
        buffer_sprintf(ob, "> sub(\"%s\") success\n", topic);
    } else {
        buffer_sprintf(ob, "> sub(\"%s\") error\n", topic);
    }
    return ZB_SET_WR;
}

int model_unsub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_left(top, acc)) {
        buffer_sprintf(ob, "> unsub(\"%s\") success\n", topic);
    } else {
        buffer_sprintf(ob, "> unsub(\"%s\") error\n", topic);
    }
    return ZB_SET_WR;
}

int model_pub(const int fd, const char* topic, const char* msg) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc_curr = account_get_by_fd(fd);
    if (!top || !acc_curr) {
        buffer_sprintf(ob, "> pub(\"%s\") error\n", topic);
        return ZB_SET_WR;
    }
    int i;
    for (i = 0; i < top->members.size; i++) {
        struct account *acc = vector_access(&top->members, i);
        ASSERT(acc);
        struct buffer *ob_dst = fdtab[acc->fd].cb[DIR_WR].b;
        buffer_sprintf(ob_dst, "%s: %s\n", acc_curr->name, msg);
        WR_ENABLE(acc->fd);
    }
    buffer_sprintf(ob, "> pub(\"%s\") success, msg_id: %d\n", topic, id_zen("msg"));
    return ZB_SET_WR;
}

int model_topic_create(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_create(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_join(top, acc)) {
        buffer_sprintf(ob, "> sub_create(\"%s\") success\n", topic);
    } else {
        buffer_sprintf(ob, "> sub_create(\"%s\") error\n", topic);
    }
    return ZB_SET_WR;
}

int model_topic_delete(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (!topic_delete(topic)) {
        buffer_sprintf(ob, "> sub_create(\"%s\") success\n", topic);
    } else {
        buffer_sprintf(ob, "> sub_create(\"%s\") error\n", topic);
    }
    return ZB_SET_WR;
}