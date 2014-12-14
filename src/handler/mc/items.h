/* 
 * File:   items.h
 * Author: khoai
 *
 * Created on September 15, 2014, 6:17 PM
 */

#ifndef ITEMS_H
#define	ITEMS_H

#include <stdbool.h>
#include <strings.h>
#include "type/item.h"

#define ITEM_key(item) ((char*)&((item)->end[0]))

/* warning: don't use these macros with a function, as it evals its arg twice */
#define ITEM_suffix(item) ((char*) &((item)->end[0]) + (item)->nkey + 1)
#define ITEM_data(item) ((char*) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix)
#define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + 1 + (item)->nsuffix + (item)->nbytes)

#define add_delta(c,x,y,z,a)          do_add_delta(c,x,y,z,a)
#define assoc_move_next_bucket()    do_assoc_move_next_bucket()
#define conn_from_freelist()        do_conn_from_freelist()
#define conn_add_to_freelist(x)     do_conn_add_to_freelist(x)
#define suffix_from_freelist()      do_suffix_from_freelist()
#define suffix_add_to_freelist(x)   do_suffix_add_to_freelist(x)
#define defer_delete(x,y)           do_defer_delete(x,y)
#define dispatch_conn_new(x,y,z,a,b) conn_new(x,y,z,a,b,main_base)
#define dispatch_event_add(t,c)     event_add(&(c)->event, 0)
#define is_listen_thread()          1
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
#define store_item(x,y)             do_store_item(x,y)
#define thread_init(x,y)            0

#define STATS_LOCK()                /**/
#define STATS_UNLOCK()              /**/

/* See items.c */
void item_init(void);
/*@null@*/
item *do_item_alloc(char *key, const size_t nkey, const int flags, const rel_time_t exptime, const int nbytes);
void item_free(item *it);
bool item_size_ok(const size_t nkey, const int flags, const int nbytes);

int do_item_link(item *it); /** may fail if transgresses limits */
void do_item_unlink(item *it);
void do_item_remove(item *it);
void do_item_update(item *it); /** update LRU time to current and reposition */
int do_item_replace(item *it, item *new_it);

/*@null@*/
char *do_item_cachedump(const unsigned int slabs_clsid, const unsigned int limit, unsigned int *bytes);
char *do_item_stats(int *bytes);

/*@null@*/
char *do_item_stats_sizes(int *bytes);
void do_item_flush_expired(void);
item *item_get(const char *key, const size_t nkey);

item *do_item_get_notedeleted(const char *key, const size_t nkey, bool *delete_locked);
item *do_item_get_nocheck(const char *key, const size_t nkey);

#endif	/* ITEMS_H */

