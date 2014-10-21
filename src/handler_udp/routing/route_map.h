/* 
 * File:   route_map.h
 * Author: khoai
 *
 * Created on October 17, 2014, 10:25 AM
 */

#ifndef ROUTE_MAP_H
#define	ROUTE_MAP_H

#include "type/route.h"
#include "../../dbg.h"
#include <string.h>
#include <sys/types.h>

void route_register(struct route *r);
void route_unregister(struct route *r);
struct route* route_get_by_name(const char* name);
struct route* route_get_by_dst(const char* dst);

static int route_register_config(const char *name, const char *src, const char *remote) {
	// filter sploil config
	if ((strlen(name) >= ROUT_BUF_REV)
			|| (strlen(src) >= ROUT_BUF_REV)
			|| (strlen(remote) >= ROUT_BUF_REV)) {
		log_warn("config field too long");
		return 1;
	}

	struct route *rout = malloc(sizeof (struct route));
	strcpy(rout->name, name);
	strcpy(rout->dest, src);
	strcpy(rout->remote, remote);
	
	regex_t *rex = malloc(sizeof (regex_t));
	int status = regcomp(rex, src, REG_EXTENDED | REG_NEWLINE);

	// compile regex
	if (status != 0) {
		char error_message[128];
		regerror(status, rex, error_message, sizeof (error_message));
		log_info("regex error compiling '%s': %s\n", src, error_message);
		return 1;
	}

	rout->rex_dest = rex;
	if (!strcmp(remote, "*")) {
		route_register(rout);
		return 0;
	}
	
	// split remote detail
	const char *pos = strchr(remote, (int) ':');
	// filter error
	if (!pos || ((pos - remote) >= strlen(remote))) {
		free(rex);
		log_err("parse error remote '%s'", remote);
		return 1;
	}

	char host[128];
	char port[20];
	memset(host, 0, sizeof (host));
	memset(port, 0, sizeof (port));
	strncpy(host, remote, pos - remote);
	pos++;
	strncpy(port, pos, strlen(remote) - (pos - remote));
	
	unsigned int slen = sizeof (rout->s_remote);
	memset((char *) &rout->s_remote, 0, slen);
	rout->s_remote.sin_family = AF_INET;
	rout->s_remote.sin_port = htons(atoi(port));
	if (inet_aton(host, &rout->s_remote.sin_addr) == 0) {
		log_err("inet_aton host '%s': %s", host, strerror(errno));
		free(rex);
		free(rout);
		return 1;
	}

	route_register(rout);
	return 0;
}

#endif	/* ROUTE_MAP_H */

