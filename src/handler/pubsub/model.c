#include "model.h"
#include "type/fd.h"
#include "handler.h"
#include "buffer.h"
#include "fd.h"
#include "idzen.h"
#include "common/strutil.h"
#include <stdio.h>

int model_acc_create(const int fd, const char* user, const char* auth) {
    printf("[CREATE] user(\"%s\"), pass(\"%s\")\n", user, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (account_create(user, auth) < 0) {
        buffer_sprintf(ob, RED("> create user error\n"));
    } else {
        buffer_sprintf(ob, GREEN("> create user(\"%s\") success\n"), user);
    }
    return ZB_SET_WR;
}

int model_acc_auth(const int fd, const char* user, const char* auth) {
    printf("[LOGIN] user(\"%s\"), pass(\"%s\")\n", user, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct account* acc = account_get(user);
    if (acc && !strncmp(acc->auth, auth, strlen(acc->auth))) {
        buffer_sprintf(ob, GREEN("> auth user(\"%s\") success\n"), user);
        acc->fd = fd;
    } else {
        buffer_sprintf(ob, RED("> auth user error\n"));
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
        buffer_sprintf(ob, GREEN("> sub(\"%s\") success\n"), topic);
    } else {
        buffer_sprintf(ob, RED("> sub(\"%s\") error\n"), topic);
    }
    return ZB_SET_WR;
}

int model_unsub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_left(top, acc)) {
        buffer_sprintf(ob, GREEN("> unsub(\"%s\") success\n"), topic);
    } else {
        buffer_sprintf(ob, RED("> unsub(\"%s\") error\n"), topic);
    }
    return ZB_SET_WR;
}

int model_pub(const int fd, const char* topic, const char* msg) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc_curr = account_get_by_fd(fd);
    if (!top || !acc_curr) {
        buffer_sprintf(ob, RED("> pub(\"%s\") error\n"), topic);
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
    buffer_sprintf(ob, GREEN("> pub(\"%s\") success, msg_id: %d\n"), topic, id_zen("msg"));
    return ZB_SET_WR;
}

int model_topic_create(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_create(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_join(top, acc)) {
        buffer_sprintf(ob, GREEN("> sub_create(\"%s\") success\n"), topic);
    } else {
        buffer_sprintf(ob, RED("> sub_create(\"%s\") error\n"), topic);
    }
    return ZB_SET_WR;
}

int model_topic_delete(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (!topic_delete(topic)) {
        buffer_sprintf(ob, GREEN("> sub_create(\"%s\") success\n"), topic);
    } else {
        buffer_sprintf(ob, RED("> sub_create(\"%s\") error\n"), topic);
    }
    return ZB_SET_WR;
}

//--- process center

int model_process(const int fd, const char *content, char delim, int *action) {
    int ret = ZB_NOP;
    const char *cmd = content;
    const char *param = NULL;
    //--- message
    if (COMPARE(cmd, "ack")) {
        printf("[ack] %s\n", cmd);
        //
        *action = ACT_ACK;
    }

    //--- drive topic
    if (COMPARE(cmd, "create")) {
        param = cmd + 7;
        printf("[create] %s\n", param);
        ret = model_topic_create(fd, param);
        *action = ACT_CREATE;
    }
    if (COMPARE(cmd, "delete")) {
        param = cmd + 7;
        printf("[delete] %s\n", param);
        ret = model_topic_delete(fd, param);
        *action = ACT_DELETE;
    }

    //--- pubsub
    if (COMPARE(cmd, "sub")) {
        param = cmd + 4;
        printf("[sub] %s\n", param);
        ret = model_sub(fd, param);
        *action = ACT_SUB;
    }
    if (COMPARE(cmd, "unsub")) {
        param = cmd + 6;
        printf("[unsub] %s\n", param);
        ret = model_unsub(fd, param);
        *action = ACT_UNSUB;
    }

    if (COMPARE(cmd, "pub")) {
        param = cmd + 4;
        printf("[pub] %s\n", param);
        char *pos = strchr(param, ' ');
        if (pos > 0) {
            // <impl>
            *pos = 0;
            pos++;
            ret = model_pub(fd, param, pos);
        } else {
            log_warn("incorrect parameter: %s", param);
        }
        *action = ACT_PUB;
    }

    //--- user
    if (COMPARE(cmd, "login")) {
        param = cmd + 6;
        printf("[login] %s\n", param);
        const char* user = param;
        char *auth = strchr(user, delim);
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_auth(fd, user, auth);
        } else {
            ret = ZB_CLOSE;
        }
        *action = ACT_LOGIN;
    }
    if (COMPARE(cmd, "logup")) {
        param = cmd + 6;
        printf("[logup] %s\n", param);
        const char* user = param;
        char *auth = strchr(user, delim);
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_create(fd, user, auth);
        } else {
            ret = ZB_CLOSE;
        }
        *action = ACT_LOGUP;
    }
    return ret;
}

//--- util for handler

struct account* model_acc_current(const int fd) {
    return account_get_by_fd(fd);
}