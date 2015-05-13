/* 
 * File:   local_port.h
 * Author: khoai
 *
 * Created on October 20, 2014, 11:31 AM
 */

#ifndef LOCAL_PORT_H
#define	LOCAL_PORT_H

#include "common/uthash.h"

struct local_port {
    unsigned int id;
    void *context;
    UT_hash_handle hh;
};

struct local_port * register_virtual_port();
struct local_port * register_port(unsigned short port, void *context);
struct local_port * get_local_port(unsigned short id);
int unregister_port(unsigned short id);

#endif	/* LOCAL_PORT_H */

