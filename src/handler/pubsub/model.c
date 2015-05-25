#include "model.h"
#include "type/fd.h"
#include "handler.h"
#include "buffer.h"
#include "fd.h"
#include "idzen.h"
#include "common/strutil.h"
#include "type/protocols.h"
#include <stdio.h>

#define IS_SUCCESS(er) ((er) == C_SUCCESS)

int model_acc_create(const int fd, const char* user, const char* auth) {
    // printf("[CREATE] user(\"%s\"), pass(\"%s\")\n", user, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (account_create(user, auth) < 0) {
        return C_FAIL;
    } else {
        log_warn("[LOGUP] create user(\"%s\") success", user);
        return C_SUCCESS;
    }
}

int model_acc_auth(const int fd, const char* user, const char* auth) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct account* acc = account_get(user);
    if (acc && !strncmp(acc->auth, auth, strlen(acc->auth))) {
        log_warn("[LOGIN] %d : user(%s)", fd, user);
        acc->fd = fd;
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

int model_acc_offline(const int fd) {
    struct account* acc = account_get_by_fd(fd);
    if (acc) {
        log_warn("[OFFLINE] fd(%d), user(%s)", fd, acc->name);
        acc->fd = 0;
    }
    return !acc;
}

int model_sub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_join(top, acc)) {
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

int model_unsub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_left(top, acc)) {
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

int model_pub(const int fd, const char* topic, const char* msg) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc_curr = account_get_by_fd(fd);
    if (!top || !acc_curr) {
        return C_FAIL;
    }
    int i;
    for (i = 0; i < top->members.size; i++) {
        struct account *acc = vector_access(&top->members, i);
        ASSERT(acc);
        if (acc->fd) {
            static char bdc_msg[4096] = {0};
            int ret = snprintf(bdc_msg, sizeof (bdc_msg), "%s: %s\n", acc_curr->name, msg);
            if (ret) {
                struct listener *listen = (struct listener *) fdtab[acc->fd].owner;
                ASSERT(listen);
                ASSERT(listen->handler->user_data);
                struct on_event_cb_setting *on_event = listen->handler->user_data;
                on_event->on_response(acc->fd, ACT_PUB, bdc_msg, ret);
            }
        }
    }
    return C_SUCCESS;
}

int model_topic_create(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_create(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_join(top, acc)) {
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

int model_topic_delete(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (!topic_delete(topic)) {
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

//--- process center

int model_process(const int fd, const char *content, char delim, int *action) {
    int state = ZB_SET_WR;
    int ret = C_SUCCESS;
    const char *cmd = content;
    const char *param = NULL;
    *action = 0;
    //--- message
    if (COMPARE(cmd, "ack")) {
        log_dbg("[ack] %s\n", cmd);
        // <impl>
        *action = ACT_ACK;
    }

    //--- drive topic
    if (COMPARE(cmd, "create")) {
        param = cmd + 7;
        log_dbg("[create] %s\n", param);
        ret = model_topic_create(fd, param);
        *action = ACT_CREATE;
    }
    if (COMPARE(cmd, "delete")) {
        param = cmd + 7;
        log_dbg("[delete] %s\n", param);
        ret = model_topic_delete(fd, param);
        *action = ACT_DELETE;
    }

    //--- pubsub
    if (COMPARE(cmd, "sub")) {
        param = cmd + 4;
        log_dbg("[sub] %s\n", param);
        ret = model_sub(fd, param);
        state = ZB_NOP;
        // <impl>
        *action = ACT_SUB;
    }
    if (COMPARE(cmd, "unsub")) {
        param = cmd + 6;
        log_dbg("[unsub] %s\n", param);
        ret = model_unsub(fd, param);
        *action = ACT_UNSUB;
    }

    if (COMPARE(cmd, "pub")) {
        param = cmd + 4;
        log_dbg("[pub] %s\n", param);
        char *pos = strchr(param, delim);
        if (pos > 0) {
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
        log_dbg("[login] %s\n", param);
        const char* user = param;
        char *auth = strchr(user, delim);
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_auth(fd, user, auth);
        } else {
            state = ZB_CLOSE;
        }
        *action = ACT_LOGIN;
    }
    if (COMPARE(cmd, "logup")) {
        param = cmd + 6;
        log_dbg("[logup] %s\n", param);
        const char* user = param;
        char *auth = strchr(user, delim);
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_create(fd, user, auth);
        } else {
            state = ZB_CLOSE;
        }
        *action = ACT_LOGUP;
    }

    //--- response process
    if (*action != ACT_SUB && ret == C_SUCCESS) return state;
    static char success[] = GREEN("success\n");
    static char error[] = RED("error\n");
    static char error_incorrect[] = RED("incorrect param\n");
    sprintf(error_incorrect, "incorrect param: %s\n", content);
    char *rep = ret == C_SUCCESS ? success : error;
    if (*action == 0) rep = error_incorrect;
    struct listener *listen = (struct listener *) fdtab[fd].owner;
    ASSERT(listen);
    ASSERT(listen->handler->user_data);
    struct on_event_cb_setting *on_event = listen->handler->user_data;
    on_event->on_response(fd, *action, rep, strlen(rep));
    return state;
}

//--- util for handler

struct account* model_acc_current(const int fd) {
    return account_get_by_fd(fd);
}