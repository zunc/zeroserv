/* 
 * File:   buffer.h
 * Author: khoai
 *
 * Created on August 26, 2014, 4:49 PM
 */

#ifndef BUFFER_H
#define	BUFFER_H

#include "type/buffer.h"

inline static int buffer_empty(struct buffer *b) {
	return (b->r == b->curr);
}

inline static void buffer_reset(struct buffer *b) {
	b->curr = b->r = b->buff;
	b->total = sizeof(b->buff);
}

inline static int buffer_remain_read(struct buffer *b) {
	return (b->r - b->curr);
}

inline static int buffer_remain_write(struct buffer *b) {
	return (b->total - (b->r - b->buff));
}

inline static int buffer_write_bytes(struct buffer *b) {
	return (b->r - b->buff);
}

inline static int buffer_read_bytes(struct buffer *b) {
	return (b->curr - b->buff);
}

#endif	/* BUFFER_H */

