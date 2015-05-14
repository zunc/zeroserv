#include <stdlib.h>
#include <string.h>
#include "handler.h"
#include "idzen.h"

#define USE_DB 0

#if !USE_DB
static struct list accounts = LIST_HEAD_INIT(accounts);
#else

#endif 

int account_create(const char *name, const char *auth) {
    struct account *acc = account_get(name);
    if (acc) return -1;
#if !USE_DB
    struct account *new_acc = malloc(sizeof (struct account));
    new_acc->name = strdup(name);
    new_acc->auth = strdup(auth);
    new_acc->id = id_zen("account");
    new_acc->created_time = new_acc->last_active = 0;
    LIST_ADDQ(&accounts, &new_acc->list);
    return new_acc->id;
#endif
}

struct account* account_get(const char* name) {
    struct account *acc;
#if !USE_DB

    list_for_each_entry(acc, &accounts, list) {
        if (!strcmp(acc->name, name))
            return acc;
    }
#endif
    return 0;
}

struct account* account_get_by_fd(const int fd) {
    struct account *acc;
#if !USE_DB

    list_for_each_entry(acc, &accounts, list) {
        if (acc->id == fd)
            return acc;
    }
#endif
    return 0;
}

int account_auth(const char *name, const char *auth) {
    struct account *acc = account_get(name);
    if (!acc) return -1;
    return strncmp(acc->auth, auth, strlen(acc->auth));
}