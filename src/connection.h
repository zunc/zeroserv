/* 
 * File:   connection.h
 * Author: khoai
 *
 * Created on August 19, 2014, 6:01 PM
 */

#ifndef CONNECTION_H
#define	CONNECTION_H

#include "type/buffer.h"

struct connection {
	int fd;
	void *owner; // listener
	struct buffer *ib, *ob;
	
};

#endif	/* CONNECTION_H */

