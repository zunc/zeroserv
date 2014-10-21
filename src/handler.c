#include <stdlib.h>
#include "handler.h"
#include "common/mini-clist.h"

static struct list handlers = LIST_HEAD_INIT(handlers);

void handler_register(struct handler *hd) {
	LIST_ADDQ(&handlers, &hd->list);
}

void handler_unregister(struct handler *hd) {
	LIST_DEL(&hd->list);
	LIST_INIT(&hd->list);
}

struct handler* handler_get(const char* name) {
	struct handler *hd;

	list_for_each_entry(hd, &handlers, list) {
		if (!strcmp(hd->name, name))
			return hd;
	}
	return 0;
}