/* 
 * File:   model.h
 * Author: khoai
 *
 * Created on May 13, 2015, 3:47 PM
 */

#ifndef MODEL_H
#define	MODEL_H

enum ACTION {
    ACT_ACK,
    ACT_CREATE,
    ACT_DELETE,
    ACT_SUB,
    ACT_UNSUB,
    ACT_PUB,
    ACT_LOGIN,
    ACT_LOGUP
};

int model_acc_create(const int fd, const char* acc, const char* auth);
int model_acc_auth(const int fd, const char* acc, const char* auth);
int model_acc_offline(const int fd);

int model_topic_create(const int fd, const char* topic);
int model_sub(const int fd, const char* topic);
int model_unsub(const int fd, const char* topic);
int model_pub(const int fd, const char* topic, const char *msg);
int model_process(const int fd, const char *content, char delim, int *action);
struct account* model_acc_current(const int fd);

#endif	/* MODEL_H */

