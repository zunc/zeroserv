/* 
 * File:   buff.h
 * Author: khoai
 *
 * Created on August 19, 2014, 11:55 AM
 */

#ifndef TYPE_BUFFER_H
#define	TYPE_BUFFER_H

struct buffer {
    char *r; // data pos
    char *curr; // process pointer
    size_t total; // total size
    char *buff; // original buffer
};

#endif	/* TYPE_BUFFER_H */

