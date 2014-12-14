#include "mineip.h"
#include "../../common/mini-clist.h"
#include "../../dbg.h"
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>

static struct list mineip = LIST_HEAD_INIT(mineip);

// ipv4

struct mip {
    unsigned int ip;
    char name[64];
    struct list list; // list
};

unsigned int hton(const char *host) {
    //	struct in_addr addr;
    unsigned int addr;
    inet_aton(host, &addr);
    return addr;
}

int mineip_init() {
    struct ifaddrs *ifaddr, *ifa;
    int s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        log_err("getifaddrs");
        return 1;
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        s = getnameinfo(ifa->ifa_addr, sizeof (struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s >= 0) {
            struct mip *ip = malloc(sizeof (struct mip));
            if (!ip) {
                log_fatal("malloc. fail");
            }
            log_info(" - interface : <%s>, address : <%s>", ifa->ifa_name, host);
            inet_aton(host, &ip->ip);
            strcpy(ip->name, ifa->ifa_name);
            LIST_ADDQ(&mineip, &ip->list);
        }
    }

    freeifaddrs(ifaddr);

    return 0;
}

int mineip_deinit() {
    // free
    //
}

unsigned int is_mine_ip(unsigned int ip) {
    struct mip *mip;

    list_for_each_entry(mip, &mineip, list) {
        if (mip->ip == ip)
            return 1;
    }
    return 0;
}

// get mine ip by NIC name

unsigned int get_ip_nic(const char *name) {
    struct mip *mip;

    list_for_each_entry(mip, &mineip, list) {
        if (!strcmp(mip->name, name))
            return mip->ip;
    }
    return 0;
}