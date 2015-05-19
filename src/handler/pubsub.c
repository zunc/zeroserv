#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <libconfig.h>

#include "pubsub.h"
#include "buffer.h"
#include "protos.h"
#include "dbg.h"
#include "fd.h"
#include "common/mini-clist.h"
#include "common.h"
#include "handler.h"
#include "common/ticks.h"
#include "common/time.h"
#include "pubsub/handler.h"
#include "pubsub/model.h"

#define HTTP_BUFFER_SIZE 1024

struct pubsub_stat _stat;

int psub_request_accept() {
    if (_stat.conn >= _stat.max_conn) {
        log_info("[pubsub] connection reach max(%ld)", _stat.max_conn);
        return ZB_FAIL;
    }
    return ZB_NOP;
}

int psub_accept(int nfd) {
    struct buffer *ib = (struct buffer*) malloc(sizeof (struct buffer));
    buffer_init(ib, HTTP_BUFFER_SIZE);
    fdtab[nfd].cb[DIR_WR].b = ib;
    struct buffer *ob = (struct buffer*) malloc(sizeof (struct buffer));
    buffer_init(ob, HTTP_BUFFER_SIZE);
    fdtab[nfd].cb[DIR_RD].b = ob;
    printf("[%*d] connected\n", 4, nfd);
    if (_stat.conn >= _stat.max_conn) {
        log_info("[pubsub] connection reach max(%ld)", _stat.max_conn);
        return ZB_FAIL;
    }
    _stat.conn++;
    return ZB_NOP;
}

int psub_disconnect(int fd) {
    log_info("[%*d] disconnect", 4, fd);
    struct buffer *b = fdtab[fd].cb[DIR_RD].b;
    buffer_free(b);
    fdtab[fd].cb[DIR_RD].b = NULL;
    _stat.conn--;
    model_acc_offline(fd);
    return ZB_NOP;
}

void format_msg(char *msg, int size) {
    int idx = 0;
    for (idx = 0; idx < size; idx++) {
        if ((*msg == '\n') || (*msg == '\r')) {
            *msg = '\0';
            break;
        }
        msg++;
    }
}

#define COMPARE(data, str) strncmp(ib->curr, str, strlen(str))

int process_plain_text(int fd, struct buffer* ib) {
    // implement follow api: https://cloud.google.com/pubsub/reference/rest/

    int ret = ZB_NOP;
    // protocol parse: text protocol
    int len = buffer_remain_read(ib);
    format_msg(ib->curr, len);

    const char *cmd = ib->curr;
    const char *param = NULL;
    //--- message
    if (!COMPARE(cmd, "ack ")) {
        printf("[ack] %s\n", cmd);
        //
    }

    //--- drive topic
    if (!COMPARE(cmd, "create ")) {
        param = cmd + 7;
        printf("[create] %s\n", param);
        ret = model_topic_create(fd, param);
    }
    if (!COMPARE(cmd, "delete ")) {
        param = cmd + 7;
        printf("[delete] %s\n", param);
        ret = model_topic_delete(fd, param);
    }

    //--- pubsub
    if (!COMPARE(cmd, "sub ")) {
        param = cmd + 4;
        printf("[sub] %s\n", param);
        ret = model_sub(fd, param);
    }
    if (!COMPARE(cmd, "unsub ")) {
        param = cmd + 6;
        printf("[unsub] %s\n", param);
        ret = model_unsub(fd, param);
    }

    if (!COMPARE(ib->curr, "pub ")) {
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
    }

    //--- user
    if (!COMPARE(ib->curr, "login ")) {
        param = cmd + 6;
        printf("[login] %s\n", param);
        const char* user = param;
        char *auth = strchr(user, ' ');
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_auth(fd, user, auth);
        } else {
            ret = ZB_CLOSE;
        }
    }
    if (!COMPARE(ib->curr, "logup ")) {
        param = cmd + 6;
        printf("[logup] %s\n", param);
        const char* user = param;
        char *auth = strchr(user, ' ');
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_create(fd, user, auth);
        } else {
            ret = ZB_CLOSE;
        }
    }
    buffer_reset(ib);
    return ret;
}

int psub_read(int fd) {
    struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
    int remain = buffer_remain_write(ib);
    int n = recv(fd, ib->r, remain, 0);
    int ret = ZB_NOP;
    // protocol parse
    if (n > 0) {
        // recv byte
        ib->r += n;
        //log_dbg("recv: %d", n);
        //ret |= ZB_SET_WR;
        *ib->r = 0;
        ret = process_plain_text(fd, ib);
    } else {
        log_dbg("recv fail: %d", n);
        printf("[%*d]disconnect\n", 4, fd);
        return ZB_CLOSE;
    }
    return ret;
}

int psub_write(int fd) {
    log_dbg("psub_write");
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    int remain = buffer_remain_read(ob);
    int n = send(fd, ob->curr, remain, 0);
    if (n > 0) {
        ob->curr += n;
        if (buffer_empty(ob)) {
            buffer_reset(ob);
            return ZB_CLOSE_WR;
        } else {
            return ZB_SET_WR;
        }
    } else {
        log_info("send: (%d)", n);
        return ZB_CLOSE;
    }
#if RUN_ONCE
    return ZB_CLOSE;
#endif
    return ZB_NOP;
}

int psub_load_config(void *config) {
    const config_setting_t *psub_cfg = (const config_setting_t*) config;
    // get routing
    if (!config_setting_lookup_int64(psub_cfg, "max_conn", (long long *) &_stat.max_conn)) {
        log_fatal("handler(%s) incorrect config", handler_pubsub.name);
    } else {
        log_info("%s{max_conn(%ld)}", handler_pubsub.name, _stat.max_conn);
    }
    return 0;
}

__attribute__((constructor))
static void __psub_handler_init(void) {
    _stat.conn = 0;
    handler_register(&handler_pubsub);
}