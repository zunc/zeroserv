#include "cfg.h"
#include "config.h"
#include "dbg.h"
#include "global.h"
#include "handler.h"
#include "proto_tcp.h"
#include "proto_udp.h"

#include <stdlib.h>
#include <libconfig.h>

#include "handler/echo.h"
#include "handler/http.h"
#include "handler/memcached.h"
#include "handler_udp/echo_udp.h"
#include "handler_udp/routing.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define MAX_INSTANCE 64
char* instance[MAX_INSTANCE];
int ist_cnt;

int config_load(const char* file) {
	// init
	int i = 0;
	ist_cnt = 0;
	global.maxsock = 20000;
	global.nbproc = 1;

	config_t cfg;
	config_setting_t *_global, *_tcp, *_udp;
	const char *str;

	config_init(&cfg);

	if (!config_read_file(&cfg, file)) {
		log_fatal("config_read_file(%s):%d: %s\n", file,
				config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
	}

	// global
	_global = config_lookup(&cfg, "global");
	if (_global) {
		config_setting_lookup_int(_global, "maxsock", &global.maxconn);
		config_setting_lookup_int(_global, "nbproc", &global.nbproc);
	}

	// tcp
	_tcp = config_lookup(&cfg, "tcp");
	if (_tcp) {
		int count = config_setting_length(_tcp);
		int i;
		for (i = 0; i < count; ++i) {
			config_setting_t *handler_cfg = config_setting_get_elem(_tcp, i);

			char *name = handler_cfg->name;
			struct handler* handler = handler_get(name);
			if (!handler) {
				log_fatal("handler not exit: %s", name);
			}

			int port;
			if (!config_setting_lookup_int(handler_cfg, "port", &port)) {
				log_fatal("handler(%s) incorrect config", name);
			} else {
				// extend config load for handler
				if (handler->load_config)
					handler->load_config(handler_cfg);
				char *sname = strdup(name);
				tcp_create_listener(sname, port, handler);
				// extend config load for handler
				//
			}
		}
	}

	// udp
	_udp = config_lookup(&cfg, "udp");
	if (_tcp) {
		int count = config_setting_length(_udp);
		int i;
		for (i = 0; i < count; ++i) {
			config_setting_t *handler_cfg = config_setting_get_elem(_udp, i);

			char *name = handler_cfg->name;
			struct handler* handler = handler_get(name);
			if (!handler) {
				log_fatal("handler not exit: %s", name);
			}

			int port;
			if (!config_setting_lookup_int(handler_cfg, "port", &port)) {
				log_fatal("handler(%s) incorrect config", name);
			} else {
				// extend config load for handler
				if (handler->load_config)
					handler->load_config(handler_cfg);
				udp_create_listener(strdup(name), port, handler);
			}
		}
	}

	config_destroy(&cfg);
	return 0;
}