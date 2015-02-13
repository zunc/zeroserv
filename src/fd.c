/*
 *
 * Copyright 2000-2009 Willy Tarreau <w@1wt.eu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fd.h"
#include "dbg.h"
#include "glob.h"
#include "global.h"

struct fdtab *fdtab = NULL; /* array of all the file descriptors */
//struct fdinfo *fdinfo = NULL;   /* less-often used infos for file descriptors */
int maxfd; /* # of the highest fd + 1 */
int totalconn; /* total # of terminated sessions */
int actconn; /* # of active sessions */

void fd_delete(int fd) {
    EV_FD_CLO(fd);
    close(fd);
    fdtab[fd].state = FD_STCLOSE;

    while ((maxfd - 1 >= 0) && (fdtab[maxfd - 1].state == FD_STCLOSE))
        maxfd--;
}

void fd_insert(int fd) {
    //	fdtab[fd].ev = 0;
    if (fd + 1 > maxfd)
        maxfd = fd + 1;
}

int fd_init(int maxsock) {
    int i;
    fdtab = (struct fdtab *) malloc(sizeof (struct fdtab) * maxsock);
    if (!fdtab) {
        log_fatal("malloc fail: maxsock(%d)\n", maxsock);
        return -1;
    }

    for (i = 0; i < global.maxconn; i++) {
        fdtab[i].state = FD_STCLOSE;
    }
    return maxsock;
}