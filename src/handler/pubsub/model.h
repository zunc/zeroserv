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

#endif	/* MODEL_H */

