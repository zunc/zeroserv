/* 
 * File:   protos.h
 * Author: khoai
 *
 * Created on August 18, 2014, 10:24 AM
 */

#ifndef PROTOS_H
#define	PROTOS_H

#include "type/protocols.h"

void protocol_register(struct protocol *proto);
void protocol_unregister(struct protocol *proto);
int protocol_bind_all(void);
int protocol_unbind_all(void);
int protocol_enable_all(void);

#endif	/* PROTOS_H */

