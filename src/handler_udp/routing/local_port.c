/*
 * implement a internal [virtual] port for routing
 */
#include "local_port.h"
#include "common/time.h"

#define MAX_PORT (2^16 - 1)
unsigned short _port = 0;
unsigned short cur_port = 0;
struct local_port *local_ports = NULL;

// register a virtual local port

struct local_port * register_virtual_port(void *context) {
    //<!> danger function, consider optimal
    if (cur_port == MAX_PORT) {
        return NULL;
    }

    while (1) {
        unsigned short id = ++_port;
        struct local_port *port = NULL;
        HASH_FIND_INT(local_ports, &id, port);
        if (!port) {
            struct local_port *ele = malloc(sizeof (struct local_port));
            ele->id = id;
            ele->context = context;
            HASH_ADD_INT(local_ports, id, ele);
            cur_port++;
            return ele;
        }
    }
}

// register real port

struct local_port * register_port(unsigned short port, void *context) {
    if (cur_port == MAX_PORT) {
        return NULL;
    }

    unsigned short id = port;
    struct local_port *pcheck = NULL;
    HASH_FIND_INT(local_ports, &id, pcheck);
    if (!pcheck) {
        struct local_port *ele = malloc(sizeof (struct local_port));
        ele->id = id;
        ele->context = context;
        HASH_ADD_INT(local_ports, id, ele);
        cur_port++;
        return ele;
    } else {
        return NULL;
    }
}

int unregister_port(unsigned short id) {
    struct local_port *port = NULL;
    HASH_FIND_INT(local_ports, &id, port);
    if (port) {
        HASH_DEL(local_ports, port);
        if (port)
            free(port);
        cur_port--;
        return 1;
    }
    return 0;
}

struct local_port * get_local_port(unsigned short id) {
    struct local_port *port = NULL;
    HASH_FIND_INT(local_ports, &id, port);
    return port;
}
