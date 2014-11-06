/* 
 * File:   slab.h
 * Author: khoai
 *
 * Created on November 6, 2014, 3:29 PM
 */

#ifndef SLABS_H
#define	SLABS_H

// ref: memcache 1.2.6
void slabs_init(const size_t limit, const double factor, const bool prealloc);

/**
 * Given object size, return id to use when allocating/freeing memory for object
 * 0 means error: can't store such a large object
 */

unsigned int slabs_clsid(const size_t size);

/** Allocate object of given length. 0 on error */ /*@null@*/
void *do_slabs_alloc(const size_t size, unsigned int id);

/** Free previously allocated object */
void do_slabs_free(void *ptr, size_t size, unsigned int id);

/** Fill buffer with stats */ /*@null@*/
char* do_slabs_stats(int *buflen);

/* Request some slab be moved between classes
  1 = success
   0 = fail
   -1 = tried. busy. send again shortly. */
int do_slabs_reassign(unsigned char srcid, unsigned char dstid);


#endif	/* SLABS_H */

