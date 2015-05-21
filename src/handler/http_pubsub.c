#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <libconfig.h>

#include "http_pubsub.h"
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
#include "http/http_parser.h"
#include "http/http_response.h"
#include "common/strutil.h"
#include "http/cookie.h"

#define HTTP_BUFFER_SIZE 1024

struct http_pubsub_stat _stat;
http_parser_settings settings;
#define SESSION "zchat"

int http_psub_request_accept() {
    if (_stat.conn >= _stat.max_conn) {
        log_info("[http_pubsub] connection reach max(%ld)", _stat.max_conn);
        return ZB_FAIL;
    }
    return ZB_NOP;
}

int http_on_url(http_parser* parser, const char *at, size_t length) {
    int fd = (int) (long) parser->data;
    struct session *sess = fdtab[fd].context;
    FREE(sess->url);
    sess->url = strndup(at, length);
    return 0;
}

int http_on_header_field(http_parser* parser, const char *at, size_t length) {
    int fd = (int) (long) parser->data;
    struct session *sess = fdtab[fd].context;
    if (COMPARE(at, "Cookie")) {
        const char *name = at + length + 2;
        if (COMPARE(name, SESSION)) {
            const char *pos = strchr(name, '\r');
            FREE(sess->cookie);
            if (pos > 0) sess->cookie = strndup(name, pos - name);
        }
    }
    return 0;
}

int http_on_msg_complete(http_parser* parser) {
    //log_info("http_on_msg_complete");
    //--- get
    int fd = (int) (long) parser->data;
    static char buf[4096] = {0};
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct session *sess = fdtab[fd].context;
    struct http_response *rep = NULL;

    //--- get session by cookie
    if (sess->cookie) {
        // in a session
        char *cookie_val = strchr(sess->cookie, '=');
        if (cookie_val) {
            struct account *acc = cookie_get_content(++cookie_val);
            if (acc) acc->fd = fd;
        }
    }

    // request: action/topic[/msg]
    // ex: pub news message
    char *param = strndup(sess->url + 1, strlen(sess->url + 1));
    int action = -1;
    int ret = model_process(fd, param, '/', &action);

    //--- http repsonse proc
    rep = http_create_response(HTTP_ERR_OK, MIME_TEXT,
        ob->curr, buffer_remain_read(ob));
    struct account *acc = model_acc_current(fd);
    char cookie_val[1024];
    if (acc && action == ACT_LOGIN) {
        char *cookie_name = strndup(cookie_put_content(acc), 10);
        sprintf(cookie_val, SESSION "=%s; Path=/", cookie_name);
        http_add_header_field(rep, "Set-Cookie", cookie_val);
        log_warn("cookie: %s", cookie_name);
        free(cookie_name);
    }
    int buf_len = http_msg_response(buf, sizeof (buf), rep);
    buffer_reset(ob);
    buffer_write(ob, buf, buf_len);
    WR_ENABLE(fd);

    if (ret & ZB_SET_WR) WR_ENABLE(fd);
    if (ret & ZB_SET_RD) RD_ENABLE(fd);

    //--- free
    free(param);
    http_destroy(rep);

    return 0;
}

void init_parser() {
    settings.on_url = http_on_url;
    settings.on_header_field = http_on_header_field;
    settings.on_message_complete = http_on_msg_complete;
}

int http_psub_accept(int nfd) {
    //    printf("[%*d] connected\n", 4, nfd);
    if (_stat.conn >= _stat.max_conn) {
        log_info("[http_pubsub] connection reach max(%ld)", _stat.max_conn);
        return ZB_FAIL;
    }
    _stat.conn++;

    //--- init
    http_parser *req = malloc(sizeof (http_parser));
    http_parser_init(req, HTTP_REQUEST);
    req->data = (void*) (long) nfd;
    fdtab[nfd].cb[DIR_RD].parser = req;

    struct buffer *ob = (struct buffer*) malloc(sizeof (struct buffer));
    buffer_init(ob, HTTP_BUFFER_SIZE);
    fdtab[nfd].cb[DIR_WR].b = ob;

    struct session *sess = (struct session*) malloc(sizeof (struct session));
    sess->cookie = sess->url = NULL;
    fdtab[nfd].context = sess;
    return ZB_NOP;
}

int http_psub_disconnect(int fd) {
    //    log_info("[%*d] disconnect", 4, fd);
    _stat.conn--;
    model_acc_offline(fd);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    buffer_free(ob);
    http_parser *req = fdtab[fd].cb[DIR_WR].parser;
    free(req);
    struct session *sess = fdtab[fd].context;
    if (sess) {
        FREE(sess->cookie);
        FREE(sess->url);
        FREE(sess);
    }
    return ZB_NOP;
}

int http_psub_read(int fd) {
    static char buf[1024] = {};
    int n = recv(fd, buf, sizeof (buf), 0);
    http_parser *req = fdtab[fd].cb[DIR_RD].parser;
    int ret = ZB_NOP;
    // protocol parse
    if (n > 0) {
        int nparsed = http_parser_execute(req, &settings, buf, n);
        if (req->upgrade) {
            log_info("handle new protocol");
        } else if (nparsed != n) {
            log_info("Parse http protocol fail");
            return ZB_CLOSE;
        }
    } else {
        //        log_info("recv fail: %d", n);
        return ZB_CLOSE;
    }
    return ret;
}

int http_psub_write(int fd) {
    //    log_dbg("http_psub_write");
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

int http_psub_load_config(void *config) {
    const config_setting_t *psub_cfg = (const config_setting_t*) config;
    // get routing
    if (!config_setting_lookup_int64(psub_cfg, "max_conn", (long long *) &_stat.max_conn)) {
        log_fatal("handler(%s) incorrect config", handler_http_pubsub.name);
    } else {
        //log_info("%s{max_conn(%ld)}", handler_http_pubsub.name, _stat.max_conn);
    }
    return 0;
}

__attribute__((constructor))
static void __http_psub_handler_init(void) {
    _stat.conn = 0;
    handler_register(&handler_http_pubsub);
    init_parser();
}