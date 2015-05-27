#include "model.h"
#include "type/fd.h"
#include "handler.h"
#include "buffer.h"
#include "fd.h"
#include "idzen.h"
#include "common/strutil.h"
#include "type/protocols.h"
#include "handler/http/cookie.h"
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
        vector_insert(&acc->fds, (void*) (long) fd);
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

int model_acc_offline(const int fd) {
    struct account* acc = account_get_by_fd(fd);
    if (acc) {
        vector_remove(&acc->fds, (void*) (long) fd);
        if (!vector_size(&acc->fds)) {
            log_warn("[OFFLINE] fd(%d), user(%s)", fd, acc->name);
        }
    }
    return !acc;
}

int model_sub(const int fd, const char* topic) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct topic *top = topic_get(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top && acc && !topic_join(top, acc)) {
        topic_sub(top, fd);
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
        topic_unsub(top, fd);
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
    for (i = (vector_size(&top->fds) - 1); i >= 0; i--) {
        int cfd = (int) (long) vector_access(&top->fds, i);
        if (!fdtab[cfd].cb[DIR_WR].b) {
            // trick detect close fd
            vector_remove(&top->fds, (void*) (long) cfd);
        } else {
            static char bdc_msg[4096] = {0};
            int ret = snprintf(bdc_msg, sizeof (bdc_msg), "%s: %s", acc_curr->name, msg);
            if (ret) {
                struct listener *listen = (struct listener *) fdtab[cfd].owner;
                ASSERT(listen);
                ASSERT(listen->handler->user_data);
                struct on_event_cb_setting *on_event = listen->handler->user_data;
                on_event->on_response(cfd, ACT_REP_SUB, bdc_msg, ret);
            }
        }
    }
    return C_SUCCESS;
}

int model_topic_create(const int fd, const char* topic) {
    struct topic *top = topic_create(topic);
    struct account *acc = account_get_by_fd(fd);
    if (top) {
        topic_sub(top, fd);
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
    
//    if (top && acc && !topic_join(top, acc)) {
//        topic_sub(top, fd);
//        return C_SUCCESS;
//    } else {
//        return C_FAIL;
//    }
}

int model_topic_delete(const int fd, const char* topic) {
    if (!topic_delete(topic)) {
        return C_SUCCESS;
    } else {
        return C_FAIL;
    }
}

vector model_topic_list(const int fd) {
    return topic_list();
}

vector model_topic_list_user(const int fd, const char* topic) {
    return topic_list_user(topic);
}

//--- process center

int model_process(const int fd, const char *content, char delim, int *action) {
    int state = ZB_SET_WR;
    int ret = C_SUCCESS;
    const char *cmd = content;
    const char *param = NULL;
    vector vlist;
    *action = 0;
    //--- message
    if (COMPARE(cmd, "ack")) {
        log_dbg("[ack] %s", cmd);
        // <impl>
        *action = ACT_ACK;
    }

    //--- drive topic
    if (COMPARE(cmd, "create")) {
        param = cmd + 7;
        log_dbg("[create] %s", param);
        ret = model_topic_create(fd, param);
        *action = ACT_CREATE;
    }
    if (COMPARE(cmd, "delete")) {
        param = cmd + 7;
        log_dbg("[delete] %s", param);
        ret = model_topic_delete(fd, param);
        *action = ACT_DELETE;
    }

    if (COMPARE(cmd, "list_user")) {
        param = cmd + 10;
        log_dbg("[list_user] %s", param);
        vlist = model_topic_list_user(fd, param);
        *action = ACT_LIST_USER;
    } else if (COMPARE(cmd, "list")) {
        log_dbg("[list]");
        vlist = model_topic_list(fd);
        *action = ACT_LIST_TOPIC;
    }

    //--- pubsub
    if (COMPARE(cmd, "sub")) {
        param = cmd + 4;
        log_info("[%d][sub] %s", fd, param);
        ret = model_sub(fd, param);
        state = ZB_NOP;
        // <impl>
        *action = ACT_SUB;
    }
    if (COMPARE(cmd, "unsub")) {
        param = cmd + 6;
        log_dbg("[unsub] %s", param);
        ret = model_unsub(fd, param);
        *action = ACT_UNSUB;
    }

    if (COMPARE(cmd, "pub")) {
        param = cmd + 4;
        log_dbg("[pub] %s", param);
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
        log_dbg("[login] %s", param);
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
        log_dbg("[logup] %s", param);
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

    if (COMPARE(cmd, "cookie")) {
        log_dbg("[cookie]");
        ret = model_dbg_cookie(fd);
        *action = ACT_DBG_COOKIE;
    }
    
    //--- response process
    if (*action == ACT_SUB && ret == C_SUCCESS) return state;
    char success[] = "success";
    char error[] = "error";
    char error_incorrect[1024];
    void *rep = ret == C_SUCCESS ? success : error;
    int rep_len = 0;
    if (*action == 0) {
        sprintf(error_incorrect, "incorrect param: %s\n", content);
        rep = error_incorrect;
    }
    rep_len = strlen(rep);
    if (*action == ACT_LIST_TOPIC || *action == ACT_LIST_USER) {
        rep = &vlist;
        rep_len = sizeof (vlist);
    }

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

//--- for admin region
int model_dbg_cookie(const int fd) {
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    // stuff
    return C_SUCCESS;
}