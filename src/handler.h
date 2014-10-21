/* 
 * File:   handler.h
 * Author: khoai
 *
 * Created on September 4, 2014, 6:14 PM
 */

#ifndef HANDLER_H
#define	HANDLER_H

#include "type/proto_handler.h"

void handler_register(struct handler *proto);
void handler_unregister(struct handler *proto);
struct handler* handler_get(const char* name);

#endif	/* HANDLER_H */

