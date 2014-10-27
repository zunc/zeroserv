/* 
 * File:   rex.h
 * Author: khoai
 *
 * Created on October 17, 2014, 10:17 AM
 */

#ifndef ROUT_H
#define	ROUT_H

#include <regex.h>

#define ROUT_BUF 128

// rout node tcpv4
struct rout_tcpv4 {
	char name[ROUT_BUF];

	char source[ROUT_BUF]; // source by string
	regex_t *rsrc; // source compiled by regex 

	char remote[ROUT_BUF];	// remote by string
	uint32_t host;
	uint16_t port;
};

#endif	/* ROUT_H */

