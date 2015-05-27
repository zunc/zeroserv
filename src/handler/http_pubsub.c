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
#include "common/parson.h"

#define HTTP_BUFFER_SIZE 1024

struct http_pubsub_stat _stat;
http_parser_settings settings;
struct on_event_cb_setting http_pubsub_event;
#define SESSION "zchat"

int http_psub_request_accept() {
    if (_stat.conn >= _stat.max_conn) {
        log_info("[http_pubsub] connection reach max(%ld)", _stat.max_conn);
        return ZB_FAIL;
    }
    return ZB_NOP;
}

int http_psub_on_response(const int fd, int action, const void *content, int length) {
    // http response packet smaller 4kb
    static char buf[4096] = {0};
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    struct http_response *rep = NULL;
    rep = http_create_response(HTTP_OK, MIME_TEXT, ob->curr, buffer_remain_read(ob));
    struct account *acc = model_acc_current(fd);
    static char cookie_field[1024];
    static char cookie_val[1024];
    if (acc && action == ACT_LOGIN) {
        strncpy(cookie_val, cookie_put_content(acc), COOKIE_LEN);
        sprintf(cookie_field, SESSION "=%s; Path=/", cookie_val);
        http_add_header_field(rep, "Set-Cookie", cookie_field);
    }

    http_add_header_field(rep, "Access-Control-Allow-Methods", "GET, POST");
    http_add_header_field(rep, "Access-Control-Allow-Credentials", "true");
    http_add_header_field(rep, "Access-Control-Allow-Origin", "http://127.0.0.1:8000");
    http_add_header_field(rep, "Access-Control-Allow-Headers", "Content-Type, *");

    //--- json serialization
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    if (action == ACT_LIST_TOPIC) {
        vector *vlist = (vector*) content;
        int i;
        json_object_set_number(root_object, "size", vector_size(vlist));
        json_object_set_value(root_object, "topics", json_value_init_array());
        JSON_Array *arr = json_object_get_array(root_object, "topics");
        for (i = 0; i < vlist->size; i++) {
            struct topic *top = vector_access(vlist, i);
            ASSERT(top);
            JSON_Value *node_value = json_value_init_object();
            JSON_Object *node_object = json_value_get_object(node_value);
            json_object_set_string(node_object, "name", top->name);
            json_object_set_number(node_object, "members", vector_size(&top->members));
            json_object_set_number(node_object, "last_active", top->last_active);
            json_array_append_value(arr, node_value);
        }
    } else if (action == ACT_LIST_USER) {
        vector *vlist = (vector*) content;
        json_object_set_number(root_object, "size", vector_size(vlist));
        json_object_set_value(root_object, "members", json_value_init_array());
        JSON_Array *arr = json_object_get_array(root_object, "members");
        int i;
        buffer_sprintf(ob, "--- Members: %d user\n", vector_size(vlist));
        for (i = 0; i < vlist->size; i++) {
            struct account *acc = vector_access(vlist, i);
            ASSERT(acc);
            //buffer_sprintf(ob, "> %s : %s\n", acc->name, acc->fd ? "Online" : "Offline");
            JSON_Value *node_value = json_value_init_object();
            JSON_Object *node_object = json_value_get_object(node_value);
            json_object_set_string(node_object, "name", acc->name);
            json_object_set_boolean(node_object, "online", vector_size(&acc->fds) != 0);
            json_object_set_number(node_object, "last_active", acc->last_active);
            json_array_append_value(arr, node_value);
        }
    } else if (action == ACT_REP_SUB) {
        char *pos = strchr((char*) content, ':');
        if (pos) {
            static char username[1024], msg[1024];
            memset(username, 0, sizeof(username));
            memset(msg, 0, sizeof(msg));
            strncpy(username, content, pos - (char*) content);
            pos += 2;
            strncpy(msg, pos, MIN(length - (pos - (char*) content), sizeof(msg)));
            json_object_set_string(root_object, "user", username);
            json_object_set_string(root_object, "msg", msg);
        }
    } else {
        json_object_set_string(root_object, "msg", content);
        if (acc && action == ACT_LOGIN) {
            json_object_set_string(root_object, SESSION, cookie_val);
        }
    }

    serialized_string = json_serialize_to_string(root_value);
    rep->content = serialized_string;
    rep->length = strlen(serialized_string);
    int buf_len = http_msg_response(buf, sizeof (buf), rep);
    buffer_reset(ob);
    buffer_write(ob, buf, buf_len);

    // free
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    http_destroy(rep);
    WR_ENABLE(fd);
    return 0;
}

int http_on_url(http_parser* parser, const char *at, size_t length) {
    int fd = (int) (long) parser->data;
    struct session *sess = fdtab[fd].context;
    FREE(sess->url);
    static char buf[4096];
    memset(buf, 0, sizeof(buf));
    strncpy(buf, at, length);
    sess->url = malloc(length + 1);
    memset(sess->url, 0, length + 1);
    uri_decode(buf, sess->url);
    return 0;
}

int http_on_header_field(http_parser* parser, const char *at, size_t length) {
    int fd = (int) (long) parser->data;
    struct session *sess = fdtab[fd].context;
    if (COMPARE(at, "Cookie")) {
        const char *name = at + length + 2;
        const char *zchat = strstr(at, "zchat=");
        if (zchat) {
            name = zchat;
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
    struct session *sess = fdtab[fd].context;

    //--- get session by cookie
    if (sess->cookie) {
        // in a session
        char *cookie_val = strchr(sess->cookie, '=');
        if (cookie_val) {
            struct account *acc = cookie_get_content(++cookie_val);
            if (acc) {
                log_err("acc: %s", acc->name);
                vector_insert(&acc->fds, (void*)(long) fd);
            }
        }
    }

    // request: action/topic[/msg]
    // ex: pub news message
    char *param = strndup(sess->url + 1, strlen(sess->url + 1));
    int action = -1;
    int ret = model_process(fd, param, '/', &action);

    //--- http repsonse proc

    //WR_ENABLE(fd);

    if (ret & ZB_SET_WR) WR_ENABLE(fd);
    if (ret & ZB_SET_RD) RD_ENABLE(fd);

    //--- free
    free(param);
    return 0;
}

void init_parser() {
    //--- http-parser
    settings.on_url = http_on_url;
    settings.on_header_field = http_on_header_field;
    settings.on_message_complete = http_on_msg_complete;

    //--- pubsub
    http_pubsub_event.on_response = http_psub_on_response;
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
    log_info("[%*d] disconnect", 4, fd);
    _stat.conn--;
    model_acc_offline(fd);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    buffer_free(ob);
    fdtab[fd].cb[DIR_WR].b = NULL;
    http_parser *req = fdtab[fd].cb[DIR_WR].parser;
    free(req);
    fdtab[fd].cb[DIR_WR].parser = NULL;
    struct session *sess = fdtab[fd].context;
    if (sess) {
        FREE(sess->cookie);
        FREE(sess->url);
        FREE(sess);
    }
    fdtab[fd].context = NULL;
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
    init_parser();
    handler_http_pubsub.user_data = (void*) &http_pubsub_event;
    handler_register(&handler_http_pubsub);
}