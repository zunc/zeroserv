/* 
 * File:   buffer.h
 * Author: khoai
 *
 * Created on August 26, 2014, 4:49 PM
 */

#ifndef BUFFER_H
#define	BUFFER_H

#include "type/buffer.h"
#include "common.h"
#include <stdlib.h>
#include <stdarg.h> 

inline static int buffer_empty(struct buffer *b) {
    return (b->r == b->curr);
}

// use it care, leak memory risk

inline static void buffer_init(struct buffer *b, int total) {
    b->buff = malloc(total);
    b->total = total;
    b->curr = b->r = b->buff;
}

inline static void buffer_free(struct buffer *b) {
    FREE(b->buff);
    FREE(b);
}

inline static void buffer_reset(struct buffer *b) {
    b->curr = b->r = b->buff;
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

inline static int buffer_sprintf(struct buffer *b, const char *format, ...) {
    int ret = 0;
    int remain = buffer_remain_write(b);
    va_list args;
    va_start(args, format);
    ret = vsnprintf(b->r, remain, format, args);
    va_end(args);
    if (ret > 0) b->r += ret;
    return ret;
}

#endif	/* BUFFER_H */

