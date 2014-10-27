/* 
 * File:   proto_routing.h
 * Author: khoai
 *
 * Created on October 16, 2014, 11:38 AM
 */

#ifndef PROTO_ROUTING_H
#define	PROTO_ROUTING_H

#include "type/routing.h"

/*
 * frameSize :	4 byte		\
 * src:port :	4 + 2 byte	 |
 * dst:port :	4 + 2 byte	 |
 * seqId :		4 byte		 |
 * ack :		1 byte		 |	24 byte
 * flag :		1 byte		 |
 * checksum :	2 byte		/
 * data :		n byte		
 */

#define MAX_UDP_MSG 64000

int routing_proc_ack(union packet *pk, cb_ack cb);
int routing_parse(int fd);

#endif	/* PROTO_ROUTING_H */

