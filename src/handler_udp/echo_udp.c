#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "echo_udp.h"
#include "buffer.h"
#include "protos.h"
#include "dbg.h"
#include "common/mini-clist.h"
#include "fd.h"
#include "common.h"
#include "handler.h"

#define RUN_ONCE	0
#define UECHO_BUFFER_SIZE 4096

int echo_udp_init(struct listener *listener) {
    int fd = listener->fd;
    struct buffer *b = (struct buffer*) malloc(sizeof (struct buffer));
    fdtab[fd].cb[DIR_RD].b = fdtab[fd].cb[DIR_WR].b = b;
    buffer_init(b, UECHO_BUFFER_SIZE);
    printf("[%*d] connected\n", 4, fd);
    return 0;
}

int echo_udp_deinit(int fd) {
    struct buffer *b = fdtab[fd].cb[DIR_WR].b;
    buffer_free(b);
    return 0;
}

int echo_udp_read(int fd) {
    struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
    int remain = buffer_remain_write(ib);

    struct sockaddr sa;
    unsigned int sa_len = sizeof (sa);
    int n = recvfrom(fd, ib->r, remain, 0, &sa, &sa_len);
    if (n > 0) {
        ib->r += n;
        log_dbg("recv: %d", n);
        //EV_FD_SET(fd, DIR_WR);

        // send echo message
        if (sendto(fd, ib->curr, n, 0, (struct sockaddr*) &sa, sa_len) <= 0) {
            log_err("sendto: %s", strerror(errno));
        }
        *ib->r = 0;
        printf("[%*d] %s", 4, fd, ib->curr);
        buffer_reset(ib);

#if RUN_ONCE
        fd_delete(fd);
        echo_disconnect(fd);
#endif
    } else {
        log_dbg("recv fail: %d", n);
        printf("[%*d]disconnect\n", 4, fd);
        fd_delete(fd);
        echo_disconnect(fd);
    }
    return 0;
}

int echo_udp_write(int fd) {
    log_dbg("echo_udp_write");
    return 0;
}

__attribute__((constructor))
static void __echo_handler_init(void) {
    handler_register(&handler_echo_udp);
}