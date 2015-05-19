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

int process_plain_text(int fd, struct buffer* ib) {
    int ret = ZB_NOP;
    // protocol parse: text protocol
    int len = buffer_remain_read(ib);
    format_msg(ib->curr, len);
    if (!strncmp(ib->curr, "sub ", 4)) {
        printf("[SUB] %s\n", ib->curr + 4);
        const char* topic = ib->curr + 4;
        ret = model_sub(fd, topic);
    } else if (!strncmp(ib->curr, "pub ", 4)) {
        printf("[PUB] %s\n", ib->curr + 4);
    } else if (!strncmp(ib->curr, "subcre ", 7)) {
        printf("[SUB_CREATE] %s\n", ib->curr + 7);
        ret = model_subcreate(fd, ib->curr + 7);
    } else if (!strncmp(ib->curr, "log ", 4)) {
        printf("[LOGIN] %s\n", ib->curr + 4);
        const char* user = ib->curr + 4;
        char *auth = strchr(user, ' ');
        if (auth) {
            *auth = 0;
            auth++;
            ret = model_acc_auth(fd, user, auth);
        } else {
            ret = ZB_CLOSE;
        }

    } else if (!strncmp(ib->curr, "cre ", 4)) {
        printf("[CREATE] %s\n", ib->curr + 4);
        const char* user = ib->curr + 4;
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