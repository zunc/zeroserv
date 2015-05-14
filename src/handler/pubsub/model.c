#include "model.h"
#include "type/fd.h"
#include "handler.h"
#include <stdio.h>

int model_acc_create(const int fd, const char* acc, const char* auth) {
    printf("[CREATE] user(\"%s\"), pass(\"%s\")\n", acc, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (account_create(acc, auth) < 0) {
        sprintf(ob->curr, "> create user error\n");
    } else {
        sprintf(ob->curr, "> create user(\"%s\") success\n", acc);
    }
    ob->r += strlen(ob->curr);
    return ZB_SET_WR;
}

int model_acc_auth(const int fd, const char* acc, const char* auth) {
    printf("[LOGIN] user(\"%s\"), pass(\"%s\")\n", acc, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct account* acc = account_get(acc);
    if (acc && !strncmp(acc->auth, auth, strlen(acc->auth))) {
        sprintf(ob->curr, "> auth user(\"%s\") success\n", acc);
        acc->fd = fd;
    } else {
        sprintf(ob->curr, "> auth user error\n");
    }
    ob->r += strlen(ob->curr);
    return ZB_SET_WR;
}

int model_sub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    sprintf(ob->curr, "> sub(\"%s\") success\n", topic);
    ob->r += strlen(ob->curr);
    return ZB_SET_WR;
}

int model_subcreate(const int fd, const char* topic) {
    struct topic *top = topic_create(topic);
    struct account *acc = account_get_by_fd(fd);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (top && acc && !topic_join(top, acc)) {
        sprintf(ob->curr, "> sub_create(\"%s\") success\n", topic);
    } else {
        sprintf(ob->curr, "> sub_create(\"%s\") error\n", topic);
    }
    ob->r += strlen(ob->curr);
    return ZB_SET_WR;
}