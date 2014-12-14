/* 
 * File:   settings.h
 * Author: khoai
 *
 * Created on September 18, 2014, 4:31 PM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H


#include "type/item.h"

struct settings {
    size_t maxbytes;
    int maxconns;
    int port;
    int udpport;
    char *inter;
    int verbose;
    rel_time_t oldest_live; /* ignore existing items older than this */
    //    bool managed;          /* if 1, a tracker manages virtual buckets */
    int evict_to_free;
    char *socketpath; /* path to unix socket if using local socket */
    int access; /* access mask (a la chmod) for unix domain socket */
    double factor; /* chunk size growth factor */
    int chunk_size;
    int num_threads; /* number of libevent threads to run */
    char prefix_delimiter; /* character that marks a key prefix (for stats) */
    int detail_enabled; /* nonzero if we're collecting detailed stats */
};

extern struct stats stats;
extern struct settings settings;

#endif	/* SETTINGS_H */

