/*
 *
 * Copyright 2000-2009 Willy Tarreau <w@1wt.eu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include "protos.h"
#include "common/mini-clist.h"

static struct list protocols = LIST_HEAD_INIT(protocols);

void protocol_register(struct protocol *proto) {
	LIST_ADDQ(&protocols, &proto->list);
}

/* Unregisters the protocol <proto>. Note that all listeners must have
 * previously been unbound.
 */
void protocol_unregister(struct protocol *proto) {
	LIST_DEL(&proto->list);
	LIST_INIT(&proto->list);
}

int protocol_bind_all(void) {
	struct protocol *proto;
	int err = 0;

	list_for_each_entry(proto, &protocols, list) {
		if (proto->bind_all)
			err |= proto->bind_all(proto);
	}
	return err;
}

int protocol_unbind_all(void) {
	struct protocol *proto;
	int err = 0;

	list_for_each_entry(proto, &protocols, list) {
		if (proto->unbind_all)
			err |= proto->unbind_all(proto);
	}
	return err;
}

int protocol_enable_all(void) {
	struct protocol *proto;
	int err = 0;

	list_for_each_entry(proto, &protocols, list) {
		if (proto->enable_all) {
			err |= proto->enable_all(proto);
		}
	}
	return err;
}