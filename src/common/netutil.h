/* 
 * File:   netutil.h
 * Author: khoai
 *
 * Created on October 21, 2014, 3:53 PM
 */

#ifndef NETUTIL_H
#define	NETUTIL_H

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

static in_port_t get_in_port(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return (((struct sockaddr_in*) sa)->sin_port);
	}
	return 0;
}

static uint32_t get_in_host(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return (((struct sockaddr_in*) sa)->sin_addr.s_addr);
	}
	return 0;
}

void ntoip(char *ip, uint32_t srcIp) {
	struct in_addr add_src;
	add_src.s_addr = srcIp;
	strcpy(ip, inet_ntoa(add_src));
}

#endif	/* NETUTIL_H */

