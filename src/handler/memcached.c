#include "memcached.h"
#include "../buffer.h"
#include "../protos.h"
#include "../dbg.h"
#include "../common/mini-clist.h"
#include "../fd.h"
#include "../common.h"
#include "../handler.h"
#include <stdlib.h>
#include <assert.h>

#include "mc/items.h"
#include "mc/stats.h"

#define MC_BUFFER_SIZE 4096

#define RD_BUFF(fd)		fdtab[fd].cb[DIR_RD].b
#define WR_BUFF(fd)		fdtab[fd].cb[DIR_WR].b
#define RD_ENABLE(fd)	EV_FD_SET(fd, DIR_RD)
#define WR_ENABLE(fd)	EV_FD_SET(fd, DIR_WR)
#define RD_DISABLE(fd)	EV_FD_CLR(fd, DIR_RD)
#define WR_DISABLE(fd)	EV_FD_CLR(fd, DIR_WR)
#define IS_RD(fd)		EV_FD_ISSET(fd, DIR_RD)
#define IS_WR(fd)		EV_FD_ISSET(fd, DIR_WR)

struct conn {
	int state;
	short ev_flags;
	short which; /** which events were just triggered */

	//	char *rbuf; /** buffer to read commands into */
	//	char *rcurr; /** but if we parsed some already, this is where we stopped */
	//	int rsize; /** total allocated size of rbuf */
	//	int rbytes; /** how much data, starting from rcur, do we have unparsed */

	//	char *wbuf;
	//	char *wcurr;
	//	int wsize;
	//	int wbytes;
	int write_and_go; /** which state to go into after finishing current write */
	void *write_and_free; /** free this memory after finishing writing */

	char *ritem; /** when we read in an item's value, it goes here */
	int rlbytes;

	/* data for the nread state */

	/**
	 * item is used to hold an item structure created after reading the command
	 * line of set/add/replace commands, but before we finished reading the actual
	 * data. The data is read into ITEM_data(item) to avoid extra copying.
	 */

	void *item; /* for commands set/add/replace  */
	int item_comm; /* which one is it: set/add/replace */

	/* data for the swallow state */
	int sbytes; /* how many bytes to swallow */
};

#define conn_data(fd) ((struct conn*) fdtab[fd].context)

typedef struct token_s {
	char *value;
	size_t length;
} token_t;

#define COMMAND_TOKEN 0
#define SUBCOMMAND_TOKEN 1
#define KEY_TOKEN 1
#define KEY_MAX_LENGTH 250

#define MAX_TOKENS 8

#define NREAD_ADD 1
#define NREAD_SET 2
#define NREAD_REPLACE 3
#define NREAD_APPEND 4
#define NREAD_PREPEND 5
#define NREAD_CAS 6

#define item_alloc(x,y,z,a,b)       do_item_alloc(x,y,z,a,b)
#define item_cachedump(x,y,z)       do_item_cachedump(x,y,z)
#define item_flush_expired()        do_item_flush_expired()
#define item_get_notedeleted(x,y,z) do_item_get_notedeleted(x,y,z)
#define item_link(x)                do_item_link(x)
#define item_remove(x)              do_item_remove(x)
#define item_replace(x,y)           do_item_replace(x,y)
#define item_stats(x)               do_item_stats(x)
#define item_stats_sizes(x)         do_item_stats_sizes(x)
#define item_unlink(x)              do_item_unlink(x)
#define item_update(x)              do_item_update(x)
#define run_deferred_deletes()      do_run_deferred_deletes()
#define slabs_alloc(x,y)            do_slabs_alloc(x,y)
#define slabs_free(x,y,z)           do_slabs_free(x,y,z)
#define slabs_reassign(x,y)         do_slabs_reassign(x,y)
#define slabs_stats(x)              do_slabs_stats(x)

enum conn_states {
	conn_listening, /** the socket which listens for connections */
	conn_read, /** reading in a command line */
	conn_write, /** writing out a simple response */
	conn_nread, /** reading in a fixed number of bytes */
	conn_swallow, /** swallowing unnecessary bytes w/o storing */
	conn_closing, /** closing this connection */
	conn_mwrite, /** writing out many items sequentially */
};

struct stats stats;

int mc_accept(int fd) {
	struct buffer *ib = (struct buffer*) malloc(sizeof (struct buffer));
	struct buffer *ob = (struct buffer*) malloc(sizeof (struct buffer));
	fdtab[fd].cb[DIR_RD].b = ib;
	fdtab[fd].cb[DIR_WR].b = ob;
	buffer_init(ib, MC_BUFFER_SIZE);
	buffer_init(ob, MC_BUFFER_SIZE);
	fdtab[fd].context = (struct conn*) malloc(sizeof (struct conn));
	conn_data(fd)->state = conn_read;
	conn_data(fd)->item = NULL;
	stats.curr_conns++;
	stats.total_conns++;
	return 0;
}

int mc_disconnect(int fd) {
	buffer_free(fdtab[fd].cb[DIR_RD].b);
	buffer_free(fdtab[fd].cb[DIR_WR].b);
	FREE(fdtab[fd].context);
	return 0;
}

static void stats_init() {
	stats.curr_items = stats.total_items = stats.curr_conns = stats.total_conns = stats.conn_structs = 0;
	stats.get_cmds = stats.set_cmds = stats.get_hits = stats.get_misses = stats.evictions = 0;
	stats.curr_bytes = stats.bytes_read = stats.bytes_written = 0;
	stats.started = time(0) - 2;
}

int mc_init(struct listener *listener) {
	stats_init();
	return 0;
}
//---

static size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens) {
	char *s, *e;
	size_t ntokens = 0;

	assert(command != NULL && tokens != NULL && max_tokens > 1);

	for (s = e = command; ntokens < max_tokens - 1; ++e) {
		if (*e == ' ') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
				*e = '\0';
			}
			s = e + 1;
		} else if (*e == '\0') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
			}

			break; /* string end */
		}
	}

	tokens[ntokens].value = *e == '\0' ? NULL : e;
	tokens[ntokens].length = 0;
	ntokens++;

	return ntokens;
}

void conn_set_state(int fd, enum conn_states state) {
	conn_data(fd)->state = state;
}

static void set_wr(int fd) {
	RD_DISABLE(fd);
	WR_ENABLE(fd);
}

static void set_rd(int fd) {
	RD_ENABLE(fd);
	WR_DISABLE(fd);
}

static void add_out_string(int fd, char *data, int len) {
	struct buffer *ob = WR_BUFF(fd);
	memcpy(ob->r, data, len);
	ob->r += len;
	set_wr(fd);
}

static void out_string(int fd, char *data) {
	struct buffer *ob = WR_BUFF(fd);
	int len = strlen(data);
	memcpy(ob->r, data, len);
	memcpy(ob->r + len, "\r\n", 2);
	ob->r += len + 2;
	set_wr(fd);
}

static inline void process_get_command(int fd, token_t *tokens, size_t ntokens) {
	char *key;
	size_t nkey;
	int i = 0;
	item *it;
	token_t *key_token = &tokens[KEY_TOKEN];
	char *suffix;
	int stats_get_cmds = 0;
	int stats_get_hits = 0;
	int stats_get_misses = 0;

	stats.get_cmds++;
	key = key_token->value;
	nkey = key_token->length;
	it = item_get(key, nkey);
	if (it) {
		add_out_string(fd, "VALUE ", 6);
		add_out_string(fd, ITEM_key(it), it->nkey);
		add_out_string(fd, ITEM_suffix(it), it->nsuffix + it->nbytes);
		stats.get_hits++;
	} else {
		log_info("miss");
		stats.get_misses++;
	}
	add_out_string(fd, "END\r\n", 5);
}

static void process_update_command(int fd, token_t *tokens, const size_t ntokens, int comm) {
	char *key;
	size_t nkey;
	int flags;
	time_t exptime;
	int vlen, old_vlen;
	uint64_t req_cas_id;
	item *it, *old_it;

	//	set_noreply_maybe(c, tokens, ntokens);
	stats.set_cmds++;
	if (tokens[KEY_TOKEN].length > KEY_MAX_LENGTH) {
		out_string(fd, "CLIENT_ERROR bad command line format");
		return;
	}

	key = tokens[KEY_TOKEN].value;
	nkey = tokens[KEY_TOKEN].length;

	flags = strtoul(tokens[2].value, NULL, 10);
	exptime = strtol(tokens[3].value, NULL, 10);
	vlen = strtol(tokens[4].value, NULL, 10);

	if (errno == ERANGE || ((flags == 0 || exptime == 0) && errno == EINVAL) || vlen < 0) {
		out_string(fd, "CLIENT_ERROR bad command line format");
		return;
	}

	it = item_alloc(key, nkey, flags, 0/*realtime(exptime)*/, vlen + 2);

	if (it == 0) {
		if (!item_size_ok(nkey, flags, vlen + 2))
			out_string(fd, "SERVER_ERROR object too large for cache");
		else
			out_string(fd, "SERVER_ERROR out of memory storing object");
		/* swallow the data line */
		conn_data(fd)->write_and_go = conn_swallow;
		conn_data(fd)->sbytes = vlen + 2;

		/* Avoid stale data persisting in cache because we failed alloc.
		 * Unacceptable for SET. Anywhere else too? */
		if (comm == NREAD_SET) {
			it = item_get(key, nkey);
			if (it) {
				item_unlink(it);
				item_remove(it);
			}
		}

		return;
	}

	conn_data(fd)->item = it;
	conn_data(fd)->ritem = ITEM_data(it);
	conn_data(fd)->rlbytes = it->nbytes;
	conn_data(fd)->item_comm = comm;
	conn_set_state(fd, conn_nread);
	RD_ENABLE(fd);
}

static void process_command(int fd, char *command) {
	token_t tokens[MAX_TOKENS];
	size_t ntokens;
	int comm;

	ntokens = tokenize_command(command, tokens, MAX_TOKENS);
	if (ntokens >= 3 &&
			((strcmp(tokens[COMMAND_TOKEN].value, "get") == 0) ||
			(strcmp(tokens[COMMAND_TOKEN].value, "bget") == 0))) {
		process_get_command(fd, tokens, ntokens);
	} else if ((ntokens == 6 || ntokens == 7) &&
			((strcmp(tokens[COMMAND_TOKEN].value, "add") == 0 && (comm = NREAD_ADD)) ||
			(strcmp(tokens[COMMAND_TOKEN].value, "set") == 0 && (comm = NREAD_SET)) ||
			(strcmp(tokens[COMMAND_TOKEN].value, "replace") == 0 && (comm = NREAD_REPLACE)) ||
			(strcmp(tokens[COMMAND_TOKEN].value, "prepend") == 0 && (comm = NREAD_PREPEND)) ||
			(strcmp(tokens[COMMAND_TOKEN].value, "append") == 0 && (comm = NREAD_APPEND)))) {
		process_update_command(fd, tokens, ntokens, comm);

	} else if (ntokens == 2 && (strcmp(tokens[COMMAND_TOKEN].value, "version") == 0)) {
		out_string(fd, "VERSION /-_-\\");
	} else if (ntokens == 2 && (strcmp(tokens[COMMAND_TOKEN].value, "stats") == 0)) {
		char temp[1024];
		pid_t pid = getpid();
		char *pos = temp;

		pos += sprintf(pos, "STAT pid %u\r\n", pid);
		//        pos += sprintf(pos, "STAT uptime %u\r\n", now);
		//        pos += sprintf(pos, "STAT time %ld\r\n", now + stats.started);
//		pos += sprintf(pos, "STAT version " VERSION "\r\n");
		pos += sprintf(pos, "STAT pointer_size %ld\r\n", 8 * sizeof (void *));
		pos += sprintf(pos, "STAT curr_items %u\r\n", stats.curr_items);
		pos += sprintf(pos, "STAT total_items %u\r\n", stats.total_items);
		pos += sprintf(pos, "STAT bytes %ld\r\n", stats.curr_bytes);
		pos += sprintf(pos, "STAT curr_connections %u\r\n", stats.curr_conns - 1); /* ignore listening conn */
		pos += sprintf(pos, "STAT total_connections %u\r\n", stats.total_conns);
//		pos += sprintf(pos, "STAT connection_structures %u\r\n", stats.conn_structs);
		pos += sprintf(pos, "STAT cmd_get %ld\r\n", stats.get_cmds);
		pos += sprintf(pos, "STAT cmd_set %ld\r\n", stats.set_cmds);
		pos += sprintf(pos, "STAT get_hits %ld\r\n", stats.get_hits);
		pos += sprintf(pos, "STAT get_misses %ld\r\n", stats.get_misses);
		pos += sprintf(pos, "STAT evictions %ld\r\n", stats.evictions);
		pos += sprintf(pos, "STAT bytes_read %ld\r\n", stats.bytes_read);
		pos += sprintf(pos, "STAT bytes_written %ld\r\n", stats.bytes_written);
		pos += sprintf(pos, "END");
		out_string(fd, temp);
	} else {
		out_string(fd, "ERROR");
	}
	return;
}

int do_store_item(item *it, int comm) {
	char *key = ITEM_key(it);
	bool delete_locked = false;
	item *old_it = do_item_get_notedeleted(key, it->nkey, &delete_locked);
	int stored = 0;

	item *new_it = NULL;
	int flags;

	if (old_it != NULL && comm == NREAD_ADD) {
		/* add only adds a nonexistent item, but promote to head of LRU */
		do_item_update(old_it);
	} else if (!old_it && (comm == NREAD_REPLACE
			|| comm == NREAD_APPEND || comm == NREAD_PREPEND)) {
		/* replace only replaces an existing value; don't store */
	} else if (delete_locked && (comm == NREAD_REPLACE || comm == NREAD_ADD
			|| comm == NREAD_APPEND || comm == NREAD_PREPEND)) {
		/* replace and add can't override delete locks; don't store */
	} else if (comm == NREAD_CAS) {
		/* validate cas operation */
		if (delete_locked)
			old_it = do_item_get_nocheck(key, it->nkey);

		if (old_it == NULL) {
			// LRU expired
			stored = 3;
		} else if (it->cas_id == old_it->cas_id) {
			// cas validates
			do_item_replace(old_it, it);
			stored = 1;
		} else {
			stored = 2;
		}
	} else {
		/*
		 * Append - combine new and old record into single one. Here it's
		 * atomic and thread-safe.
		 */

		if (comm == NREAD_APPEND || comm == NREAD_PREPEND) {

			/* we have it and old_it here - alloc memory to hold both */
			/* flags was already lost - so recover them from ITEM_suffix(it) */

			flags = (int) strtol(ITEM_suffix(old_it), (char **) NULL, 10);

			new_it = do_item_alloc(key, it->nkey, flags, old_it->exptime, it->nbytes + old_it->nbytes - 2 /* CRLF */);

			if (new_it == NULL) {
				/* SERVER_ERROR out of memory */
				if (old_it != NULL)
					do_item_remove(old_it);

				return 0;
			}

			/* copy data from it and old_it to new_it */

			if (comm == NREAD_APPEND) {
				memcpy(ITEM_data(new_it), ITEM_data(old_it), old_it->nbytes);
				memcpy(ITEM_data(new_it) + old_it->nbytes - 2 /* CRLF */, ITEM_data(it), it->nbytes);
			} else {
				/* NREAD_PREPEND */
				memcpy(ITEM_data(new_it), ITEM_data(it), it->nbytes);
				memcpy(ITEM_data(new_it) + it->nbytes - 2 /* CRLF */, ITEM_data(old_it), old_it->nbytes);
			}

			it = new_it;
		}

		/* "set" commands can override the delete lock
		   window... in which case we have to find the old hidden item
		   that's in the namespace/LRU but wasn't returned by
		   item_get.... because we need to replace it */
		if (delete_locked)
			old_it = do_item_get_nocheck(key, it->nkey);

		if (old_it != NULL)
			do_item_replace(old_it, it);
		else
			do_item_link(it);

		stored = 1;
	}

	if (old_it != NULL)
		do_item_remove(old_it); /* release our reference */
	if (new_it != NULL)
		do_item_remove(new_it);

	return stored;
}

static void complete_nread(int fd) {
	item *it = conn_data(fd)->item;
	int comm = conn_data(fd)->item_comm;
	int ret;

	stats.set_cmds++;
	char* data = ITEM_data(it);
	if (strncmp(ITEM_data(it) + it->nbytes - 2, "\r\n", 2) != 0) {
		out_string(fd, "CLIENT_ERROR bad data chunk");
	} else {
		ret = store_item(it, comm);
		if (ret == 1) {
			out_string(fd, "STORED");
		} else if (ret == 2)
			out_string(fd, "EXISTS");
		else if (ret == 3)
			out_string(fd, "NOT_FOUND");
		else
			out_string(fd, "NOT_STORED");
	}

	//	// dirty for debug
	//	char key[128];
	//	strncpy(key, ITEM_key(it), it->nbytes - 2);
	//	log_info("set: %s", key);
	item_remove(conn_data(fd)->item); /* release the c->item reference */
	conn_data(fd)->item = 0;
	RD_DISABLE(fd);
}

static int try_read_command(int fd) {
	char *el, *cont;
	struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
	assert(ib != NULL);
	if (buffer_empty(ib))
		return 0;
	el = memchr(ib->curr, '\n', buffer_remain_read(ib));
	if (!el)
		return 0;
	cont = el + 1;
	if ((el - ib->curr) > 1 && *(el - 1) == '\r') {
		el--;
	}
	*el = '\0';
	assert(cont <= ib->r);
	process_command(fd, ib->curr);
	ib->curr = cont;
	assert(ib->curr <= ib->r);
	return 1;
}

int read_bytes(int fd, int bytes) {
	struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
	int remain = buffer_remain_write(ib);
	if ((bytes <= 0) || (remain < bytes)) {
		log_fatal("bytes need recv > remain_write");
	}

	int n = recv(fd, ib->r, bytes, 0);
	if (n > 0)
		ib->r += n;
	return n;
}

int try_read_network(int fd) {
	struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
	while (1) {
		int avail = buffer_remain_write(ib);
		int res = read_bytes(fd, avail);
		if (res > 0) {
			stats.bytes_read += res;
			if (res == avail) {
				continue;
			} else {
				break;
			}
		} else {
			if (res == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) break;
			}
			conn_set_state(fd, conn_closing);
			return 1;
		}
	}
	return 1;
}

static void conn_cleanup(int fd) {
	struct conn *c = conn_data(fd);
	if (c) {
		if (c->item) {
			item_remove(c->item);
			c->item = 0;
		}
	}
}

static void conn_close(int fd) {
	conn_cleanup(fd);
	fd_delete(fd);
	mc_disconnect(fd);
	stats.curr_conns--;
	return;
}

int mc_read(int fd) {
	struct conn *c = conn_data(fd);
	struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
	int remain = buffer_remain_write(ib);
	int res;
	bool stop = false;

	while (!stop) {
		switch (c->state) {
			case conn_read:
				if (try_read_command(fd)) {
					if (IS_WR(fd)) {
						stop = true;
						break;
					} else {
						continue;
					}
				}
				if (try_read_network(fd)) {
					continue;
				}
				stop = true;
				break;
			case conn_nread:
				/* we are reading rlbytes into ritem; */
				if (c->rlbytes == 0) {
					complete_nread(fd);
					if (IS_WR(fd)) {
						stop = true;
						break;
					} else {
						continue;
					}
					break;
				}

				// read nconn
				if (buffer_remain_read(ib) > 0) {
					int remain_read = buffer_remain_read(ib);
					int tocopy = remain_read > c->rlbytes ? c->rlbytes : remain_read;
					memcpy(c->ritem, ib->curr, tocopy);
					c->ritem += tocopy;
					c->rlbytes -= tocopy;
					ib->curr += tocopy;
					break;
				}

				//---
				res = read_bytes(fd, c->rlbytes);
				if (res > 0) {
					stats.bytes_read += res;
					c->ritem += res;
					c->rlbytes -= res;
					break;
				}
				if (res <= 0) { /* end of stream */
					conn_set_state(fd, conn_closing);
					break;
				}

				stop = true;
				break;
			case conn_closing:
				log_info("[%-*d]disconnect", 4, fd);
				conn_close(fd);
				stop = true;
				break;
		}
	}
	return 0;
}

int mc_write(int fd) {
	//	log_info("mc_write");
	struct buffer *ib = fdtab[fd].cb[DIR_RD].b;
	struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
	int remain = buffer_remain_read(ob);
	int n = send(fd, ob->curr, remain, 0);
	if (n > 0) {
		ob->curr += n;
		stats.bytes_written += n;
		if (buffer_empty(ob)) {
			buffer_reset(ib);
			buffer_reset(ob);
			conn_set_state(fd, conn_read);

			WR_DISABLE(fd);
			RD_ENABLE(fd);
		}
	} else {
		conn_close(fd);
	}
	return 0;
}

__attribute__((constructor))
static void __mc_handler_init(void) {
	item_init();
	assoc_init();
	handler_register(&handler_memcached);
}