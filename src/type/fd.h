/* 
 * File:   fd.h
 * Author: khoai
 *
 * Created on August 20, 2014, 2:05 PM
 */

#ifndef TYPE_FD_H
#define	TYPE_FD_H

#include "buffer.h"

#define FD_STCLOSE	0
#define FD_STLISTEN	1
#define FD_STCONN	2
#define FD_STREADY	3
#define FD_STERROR	4

#define FD_FL_TCP               0x0001       /* socket is TCP */
#define FD_FL_TCP_NODELAY       0x0002
#define FD_FL_TCP_NOLING        0x0004       /* lingering disabled */

enum {
	DIR_RD = 0,
	DIR_WR = 1,
	DIR_CLO = 2,
	DIR_SIZE
};

// return by callback function

enum {
	ZB_NOP = 0, // on nop, keep connect
	ZB_CLOSE_RD = 0x1,
	ZB_CLOSE_WR = 0x2,
	ZB_CLOSE = ZB_CLOSE_RD | ZB_CLOSE_WR,
	ZB_SET_RD = 0x4,
	ZB_SET_WR = 0x8,
	ZB_FAIL = 0xff
};

struct fdtab {

	struct {
		int (*f)(int fd); /* read/write function */
		struct buffer *b; /* read/write buffer */
	} cb[DIR_SIZE];
	void *owner; /* the session (or proxy) associated with this fd */

	struct { /* used by pollers which support speculative polling */
		unsigned char e; /* read and write events status. 4 bits, may be merged into flags' lower bits */
		unsigned int s1; /* Position in spec list+1. 0=not in list. */
	} spec;
	unsigned short flags; /* various flags precising the exact status of this fd */
	unsigned char state; /* the state of this fd */
	unsigned char ev; /* event seen in return of poll() : FD_POLL_* */
	void *context;
};

extern struct fdtab *fdtab; /* array of all the file descriptors */
extern int maxfd; /* # of the highest fd + 1 */
extern int totalconn; /* total # of terminated sessions */
extern int actconn; /* # of active sessions */

#endif	/* TYPE_FD_H */

