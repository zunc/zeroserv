/* 
 * File:   item.h
 * Author: khoai
 *
 * Created on September 15, 2014, 6:12 PM
 */

#ifndef ITEM_H
#define	ITEM_H

#include <stdint.h>
typedef unsigned int rel_time_t;

typedef struct _stritem {
    struct _stritem *next;
    struct _stritem *prev;
    struct _stritem *h_next; /* hash chain next */
    rel_time_t time; /* least recent access */
    rel_time_t exptime; /* expire time */
    int nbytes; /* size of data */
    unsigned short refcount;
    uint8_t nsuffix; /* length of flags-and-length string */
    uint8_t it_flags; /* ITEM_* above */
    uint8_t slabs_clsid; /* which slab class we're in */
    uint8_t nkey; /* key length, w/terminating null and padding */
    uint64_t cas_id; /* the CAS identifier */
    void * end[];
    /* then null-terminated key */
    /* then " flags length\r\n" (no terminating null) */
    /* then data with terminating \r\n (no terminating null; it's binary!) */
} item;

#endif	/* ITEM_H */

