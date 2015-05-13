/* 
 * File:   rex.h
 * Author: khoai
 *
 * Created on October 17, 2014, 10:17 AM
 */

#ifndef ROUTE_H
#define	ROUTE_H

#include <regex.h>
#include <stdint.h>
#include <netinet/ip.h>
#include "common/mini-clist.h"

#define ROUT_BUF 128
#define ROUT_BUF_REV (ROUT_BUF - 1)

// route node tcpv4

struct route {
    char name[ROUT_BUF];

    char dest[ROUT_BUF]; // source by string
    regex_t *rex_dest; // source compiled by regex 

    char remote[ROUT_BUF]; // remote by string
    struct sockaddr_in s_remote;

    struct list list; // list
};

#endif	/* ROUTE_H */

