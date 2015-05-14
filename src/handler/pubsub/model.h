/* 
 * File:   model.h
 * Author: khoai
 *
 * Created on May 13, 2015, 3:47 PM
 */

#ifndef MODEL_H
#define	MODEL_H

int model_acc_create(const int fd, const char* acc, const char* auth);
int model_acc_auth(const int fd, const char* acc, const char* auth);

int model_subcreate(const int fd, const char* topic);
int model_sub(const int fd, const char* topic);
int model_unsub(const int fd, const char* topic);
int model_pub(const int fd, const char* topic, const char *msg);

#endif	/* MODEL_H */

