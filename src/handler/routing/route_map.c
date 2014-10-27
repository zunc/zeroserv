#include <stdlib.h>
#include "route_map.h"

static struct list routes = LIST_HEAD_INIT(routes);

void route_register(struct route *r) {
	LIST_ADDQ(&routes, &r->list);
}

void route_unregister(struct route *r) {
	LIST_DEL(&r->list);
	LIST_INIT(&r->list);
	if (r->rex_dest)
		free(r->rex_dest);
	if (r)
		free(r);
}

struct route* route_get_by_name(const char* name) {
	struct route *r;

	list_for_each_entry(r, &routes, list) {
		if (!strcmp(r->name, name))
			return r;
	}
	return 0;
}

struct route* route_get_by_dst(const char* dst) {
	struct route *r;
	const int n_matches = 10;
	regmatch_t m[n_matches];
	
	list_for_each_entry(r, &routes, list) {
		int no_match = regexec(r->rex_dest, dst, n_matches, m, 0);
		if (!no_match) {
			return r;
		}
	}
	return 0;
}