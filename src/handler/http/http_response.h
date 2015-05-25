/* 
 * File:   http_response.h
 * Author: khoai
 *
 * Created on May 20, 2015, 2:14 PM
 */

#ifndef HTTP_RESPONSE_H
#define	HTTP_RESPONSE_H

#include "common.h"
#include "common/mini-clist.h"
#define SERVER_NAME "zeroser"

#define MIME_TEXT "text/html; charset=UTF-8"
#define HTTP_OK         200
#define HTTP_NOT_FOUND  404

struct header_field {
    char *name;
    char *value;
};

struct http_response {
    int err_code;
    char *mime_type;
    const char *content;
    int length;
    struct vector fields;
};

struct http_response* http_create_response(int err, char *mime_type, char *content, int length) {
    struct http_response *rep = malloc(sizeof (struct http_response));
    rep->err_code = err;
    rep->mime_type = mime_type;
    rep->content = content;
    rep->length = length;
    rep->fields = vector_create(10, 1.25);
    return rep;
}

void http_destroy(struct http_response *rep) {
    struct header_field *field;
    int i;
    for (i = 0; i < vector_size(&rep->fields); i++) {
        field = vector_access(&rep->fields, i);
        free(field);
    }
    vector_destroy(&rep->fields);
}

struct http_response* http_add_header_field(struct http_response *rep, char *name, char *value) {
    struct header_field *field = malloc(sizeof (struct header_field));
    field->name = name;
    field->value = value;
    vector_insert(&rep->fields, field);
    return rep;
}

static int http_msg_response(char* buf, int max_len, struct http_response *rep) {
    ASSERT(buf);
    char *err_msg = NULL;
    // hard-code for some common code
    if (rep->err_code == 200) {
        err_msg = "OK";
    } else if (rep->err_code == 400) {
        err_msg = "Bad Request";
    } else if (rep->err_code == 401) {
        err_msg = "Unauthorized";
    } else if (rep->err_code == 403) {
        err_msg = "Forbidden";
    } else if (rep->err_code == 404) {
        err_msg = "Not Found";
    } else {
        err_msg = "Unknown";
    }

    // "Connection: close\r\n\r\n"
    int n = snprintf(buf, max_len,
        "HTTP/1.1 %d %s\r\n"
        "Server: " SERVER_NAME "\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n",
        rep->err_code, err_msg,
        rep->mime_type, rep->length);

    static char field_val[1024] = {0};
    struct header_field *field;
    int i;
    http_add_header_field(rep, "Connection", "close");
    for (i = 0; i < vector_size(&rep->fields); i++) {
        field = vector_access(&rep->fields, i);
        sprintf(field_val, "%s: %s\r\n", field->name, field->value);
        if ((max_len - 2) <= (strlen(buf) + strlen(field_val))) {
            log_warn("http_msg_response: buffer over max_len");
            return -1;
        }
        strcat(buf, field_val);
    }
    strcat(buf, "\r\n");
    int cur_len = strlen(buf);
    if (cur_len <= 0) return cur_len;
    if ((cur_len + rep->length) > max_len) return -1;
    char *pos = buf + cur_len;
    memcpy(pos, rep->content, rep->length);
    return (cur_len + rep->length);
}

#endif	/* HTTP_RESPONSE_H */

