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

#include "trouting.h"
#include "../buffer.h"
#include "../protos.h"
#include "../dbg.h"
#include "../common/mini-clist.h"
#include "../fd.h"
#include "../common.h"
#include "../handler.h"
#include "routing/tcp_router_parse.h"

struct router_setting trouter_setting;

#define TEMP_SIZE 4096
char buf_[TEMP_SIZE];

// state recv packet

enum STATE {
	ST_FRAMESIZE = 1,
	//	ST_HEADER = 2,
	ST_DATA = 3,
	ST_FINISH = 4
};

struct conn_context {
	uint8_t state;
};

int trouting_init(struct listener *listener) {
	// mineip init
	if (mineip_init()) {
		log_fatal("mineip_init. fail");
	}

	// register bind as a local port
	register_port(listener->port, NULL);
	printf("trouting_init");

	memset(buf_, 0, sizeof (buf_));
	return 0;
}

int trouting_deinit(int fd) {
	return 0;
}

int trouting_accept(int fd) {
	log_info("trouting_accept: %d", fd);
	struct conn_context *ctx = malloc(sizeof (struct conn_context));
	ctx->state = ST_FRAMESIZE;
	fdtab[fd].context = ctx;
	
	// create a timeout schedule
	// 
	return 0;
}

int trouting_disconnect(int fd) {
	log_info("trouting_disconnect: %d", fd);
	struct conn_context *ctx = fdtab[fd].context;
	FREE(ctx);
	return 0;
}

int trouting_read(int fd) {
	struct conn_context *ctx = fdtab[fd].context;

	while (1) {
		if (ctx->state == ST_FRAMESIZE) {
			uint32_t frameSize = 0;
			int n = read(fd, &frameSize, 4);
			if ((n != sizeof (frameSize) || (frameSize > trouter_setting.max_packet))) {
				fd_delete(fd);
				trouting_disconnect(fd);
				return 1;
			}

			struct buffer *ib = malloc(sizeof (struct buffer));
			buffer_init(ib, frameSize);
			fdtab[fd].cb[DIR_WR].b = ib;
			*(uint32_t*) ib->r = frameSize;
			ib->r += sizeof (uint32_t);

			ctx->state = ST_DATA;
		} else if (ctx->state == ST_DATA) {
			struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
			int remain = buffer_remain_write(ib);
			int n = read(fd, ib->r, remain);
			if (n <= 0) {
				fd_delete(fd);
				trouting_disconnect(fd);
				return 1;
			}
			
			ib->r += n;
			if (n == remain) {
				// remove task timeout schedule
				//
				int ret = tcp_router_parse(fd);
				// success recv packet
				ctx->state = ST_FRAMESIZE;
			}
			return 0;
		}
	}
	return 0;
}

int trouting_write(int fd) {
	log_dbg("routing_write");
	return 0;
}

int trouting_load_config(void *config) {
	const config_setting_t *cfg = (const config_setting_t*) config;
	// routing settings
	if (!(config_setting_lookup_int(cfg, "retry", &trouter_setting.retry)
			&& config_setting_lookup_int(cfg, "timeout", &trouter_setting.timeout)
			&& config_setting_lookup_int(cfg, "max_packet", &trouter_setting.max_packet)
			&& config_setting_lookup_string(cfg, "layer", &trouter_setting.layer)
			)) {
		log_fatal("handler(%s) incorrect settings", cfg->name);
	}
	return 0;
}

__attribute__((constructor))
static void __echo_handler_init(void) {
	handler_register(&handler_trouting);
}