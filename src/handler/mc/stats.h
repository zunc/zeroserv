/* 
 * File:   stats.h
 * Author: khoai
 *
 * Created on September 15, 2014, 6:11 PM
 */

#ifndef STATS_H
#define	STATS_H

struct stats {
    unsigned int curr_items;
    unsigned int total_items;
    uint64_t curr_bytes;
    unsigned int curr_conns;
    unsigned int total_conns;
    unsigned int conn_structs;
    uint64_t get_cmds;
    uint64_t set_cmds;
    uint64_t get_hits;
    uint64_t get_misses;
    uint64_t evictions;
    time_t started; /* when the process was started */
    uint64_t bytes_read;
    uint64_t bytes_written;
};

#endif	/* STATS_H */

