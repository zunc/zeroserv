/* 
 * File:   poller.h
 * Author: khoai
 *
 * Created on August 15, 2014, 5:35 PM
 */

#ifndef POLLER_H
#define	POLLER_H

// poll
int poll_init();
int poll_term();
int poll_do(int exp);

int __fd_is_set(const int fd, int dir);
void fd_flush_changes();
void alloc_chg_list(const int fd, int old_evt);
int __fd_set(const int fd, int dir);
int __fd_clr(const int fd, int dir);
void __fd_rem(int fd);
void __fd_clo(int fd);

#endif	/* POLLER_H */

