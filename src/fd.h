/* 
 * File:   fd.h
 * Author: khoai
 *
 * Created on August 15, 2014, 3:28 PM
 */

#ifndef FD_H
#define	FD_H

#include "type/fd.h"
#include "poller.h"

#define EV_FD_SET(fd, ev)    (__fd_set((fd), (ev)))
#define EV_FD_CLR(fd, ev)    (__fd_clr((fd), (ev)))
#define EV_FD_ISSET(fd, ev)  (__fd_is_set((fd), (ev)))
#define EV_FD_COND_S(fd, ev) (__fd_set((fd), (ev)))
#define EV_FD_COND_C(fd, ev) (__fd_clr((fd), (ev)))
#define EV_FD_REM(fd)        (__fd_rem(fd))
#define EV_FD_CLO(fd)        (__fd_clo(fd))

#define RD_ENABLE(fd)	EV_FD_SET(fd, DIR_RD)
#define WR_ENABLE(fd)	EV_FD_SET(fd, DIR_WR)
#define RD_DISABLE(fd)	EV_FD_CLR(fd, DIR_RD)
#define WR_DISABLE(fd)	EV_FD_CLR(fd, DIR_WR)
#define IS_RD(fd)		EV_FD_ISSET(fd, DIR_RD)
#define IS_WR(fd)		EV_FD_ISSET(fd, DIR_WR)
#define CLOSE_FD(fd)	EV_FD_CLO(fd, DIR_WR)

void fd_delete(int fd);
void fd_insert(int fd);
int fd_init(int maxsock);

#endif	/* FD_H */

