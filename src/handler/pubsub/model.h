/* 
 * File:   model.h
 * Author: khoai
 *
 * Created on May 13, 2015, 3:47 PM
 */

#ifndef MODEL_H
#define	MODEL_H

#include "common/vector.h"

enum ACTION {
    ACT_ACK = 1,
    ACT_CREATE,
    ACT_DELETE,
    ACT_LIST_TOPIC,
    ACT_LIST_USER,
    ACT_SUB,
    ACT_UNSUB,
    ACT_PUB,
    ACT_LOGIN,
    ACT_LOGUP,
    ACT_REP_SUB,
    
    ACT_DBG_COOKIE = 100
};

enum ECODE {
    C_SUCCESS = 0,
    C_FAIL
};

typedef int (*on_response_cb) (const int fd, int action, const void *content, int length);

struct on_event_cb_setting {
    on_response_cb on_response; 
};

int model_acc_create(const int fd, const char* acc, const char* auth);
int model_acc_auth(const int fd, const char* acc, const char* auth);
int model_acc_offline(const int fd);

int model_topic_create(const int fd, const char* topic);
int model_topic_delete(const int fd, const char* topic);
vector model_topic_list(const int fd);
vector model_topic_list_user(const int fd, const char* topic);

int model_sub(const int fd, const char* topic);
int model_unsub(const int fd, const char* topic);
int model_pub(const int fd, const char* topic, const char *msg);
int model_process(const int fd, const char *content, char delim, int *action);
struct account* model_acc_current(const int fd);

int model_dbg_cookie(const int fd);

#endif	/* MODEL_H */

