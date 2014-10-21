#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <libconfig.h>
#include <netdb.h>

#include "routing.h"
#include "../buffer.h"
#include "../protos.h"
#include "../dbg.h"
#include "../common/mini-clist.h"
#include "../fd.h"
#include "../common.h"
#include "../handler.h"
#include "routing/proto_routing.h"
#include "routing/route_map.h"
#include "routing/mineip.h"
#include "routing/local_port.h"

struct router_setting router_setting;

int routing_init(struct listener *listener) {
	// mineip init
	if (mineip_init()) {
		log_fatal("mineip_init. fail");
	}

	// register bind as a local port
	register_port(listener->port, NULL);

	// buffer init
	int fd = listener->fd;
	struct buffer *b = (struct buffer*) malloc(sizeof (struct buffer));
	fdtab[fd].cb[DIR_RD].b = fdtab[fd].cb[DIR_WR].b = b;
	buffer_reset(b);
	printf("[%*d] connected\n", 4, fd);
	return 0;
}

int routing_deinit(int fd) {
	struct buffer *b = fdtab[fd].cb[DIR_WR].b;
	FREE(b);
	return 0;
}

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

int routing_read(int fd) {
	struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
	int remain = buffer_remain_write(ib);

	struct sockaddr sa;
	unsigned int sa_len = sizeof (sa);
	int n = recvfrom(fd, ib->r, remain, 0, &sa, &sa_len);
	if (n > 0) {
		ib->r += n;
		log_dbg("recv: %d", n);

		char host[NI_MAXHOST];
		char port[NI_MAXSERV];

		int rc = getnameinfo((struct sockaddr *)&sa, sa_len, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
		if (rc == 0) printf("connection from %s %s", host, port);
		
		// process protocol
		int ret = routing_parse(fd);
		buffer_reset(ib);

		//		EV_FD_SET(fd, DIR_WR);

		//		// send echo message
		//		if (sendto(fd, ib->curr, n, 0, (struct sockaddr*) &sa, sa_len) <= 0) {
		//			log_err("sendto: %s", strerror(errno));
		//		}
		//		*ib->r = 0;
		//		printf("[%*d] %s", 4, fd, ib->curr);

	} else {
		log_dbg("recv fail: %d", n);
		printf("[%*d]disconnect\n", 4, fd);
		fd_delete(fd);
		echo_disconnect(fd);
	}
	return 0;
}

int routing_write(int fd) {
	log_dbg("routing_write");
	return 0;
}

int routing_load_config(void *config) {
	const config_setting_t *cfg = (const config_setting_t*) config;
	// get routing
	const config_setting_t *_rout = config_setting_get_member(cfg, "rout");
	if (!_rout) {
		log_fatal("[%s] not found rout detail", handler_routing.name);
	}

	// routing map load
	int count = config_setting_length(_rout);
	int i;
	for (i = 0; i < count; ++i) {
		config_setting_t *rout_node = config_setting_get_elem(_rout, i);

		const char *name, *source, *remote;
		if (!(config_setting_lookup_string(rout_node, "name", &name)
				&& config_setting_lookup_string(rout_node, "source", &source)
				&& config_setting_lookup_string(rout_node, "remote", &remote))) {
			log_fatal("handler(%s) incorrect config", name);
		} else {
			printf(" - rout (%-10s): %-25s -> %s\n", name, source, remote);
			route_register_config(name, source, remote);
		}
	}

	// routing settings
	if (!(config_setting_lookup_int(cfg, "retry", &router_setting.retry)
			&& config_setting_lookup_int(cfg, "timeout", &router_setting.timeout)
			&& config_setting_lookup_int(cfg, "max_packet", &router_setting.max_packet)
			)) {
		log_fatal("handler(%s) incorrect settings", cfg->name);
	}
	return 0;
}

__attribute__((constructor))
static void __echo_handler_init(void) {
	handler_register(&handler_routing);
}