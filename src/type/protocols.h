/* 
 * File:   protocols.h
 * Author: khoai
 *
 * Created on August 18, 2014, 10:15 AM
 */

#ifndef TYPE_PROTOCOLS_H
#define	TYPE_PROTOCOLS_H

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "../common/mini-clist.h"
#include "proto_handler.h"

#define PROTO_NAME_LEN 16

struct listener {
    int fd; /* the listen socket */
    char *name; /* */
    int port;
    int luid; /* listener universally unique ID, used for SNMP */
    int state; /* state: NEW, INIT, ASSIGNED, LISTEN, READY, FULL */
    int options; /* socket options : LI_O_* */
    //	struct licounters *counters;	/* statistics counters */
    //	struct sockaddr_storage addr;	/* the address we listen to */
    struct protocol *proto; /* protocol this listener belongs to */
    struct handler *handler;
    int nbconn; /* current number of connections on this listener */
    int maxconn; /* maximum connections allowed on this listener */
    unsigned int backlog; /* if set, listen backlog */
    struct listener *next; /* next address for the same proxy, or NULL */
    struct list proto_list; /* list in the protocol header */
    int (*accept)(int fd); /* accept() function passed to fdtab[] */
    //	struct task * (*handler)(struct task *t); /* protocol handler. It is a task */
    int *timeout; /* pointer to client-side timeout */
    void *private; /* any private data which may be used by accept() */
    unsigned int analysers; /* bitmap of required protocol analysers */
    int nice; /* nice value to assign to the instanciated tasks */

    union { /* protocol-dependant access restrictions */

        struct { /* UNIX socket permissions */
            uid_t uid; /* -1 to leave unchanged */
            gid_t gid; /* -1 to leave unchanged */
            mode_t mode; /* 0 to leave unchanged */
            int level; /* access level (ACCESS_LVL_*) */
        } ux;
    } perm;
    char *interface; /* interface name or NULL */
    int maxseg; /* for TCP, advertised MSS */

    struct {
        const char *file; /* file where the section appears */
        int line; /* line where the section appears */
        //		struct eb32_node id;	/* place in the tree of used IDs */
    } conf; /* config information */
};

struct protocol {
    char name[PROTO_NAME_LEN];
    int sock_domain; /* socket domain, as passed to socket()   */
    int sock_type; /* socket type, as passed to socket()     */
    int sock_prot; /* socket protocol, as passed to socket() */
    sa_family_t sock_family; /* socket family, for sockaddr */
    socklen_t sock_addrlen; /* socket address length, used by bind() */
    int l3_addrlen; /* layer3 address length, used by hashes */
    int (*read)(int fd);
    int (*write)(int fd);
    int (*bind_all)(struct protocol *proto);
    int (*unbind_all)(struct protocol *proto);
    int (*enable_all)(struct protocol *proto);
    struct list listeners; /* list of listeners using this protocol */
    int nb_listeners; /* number of listeners */
    struct list list;
};

#endif	/* TYPE_PROTOCOLS_H */

