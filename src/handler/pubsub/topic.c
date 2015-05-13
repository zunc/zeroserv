#include <stdlib.h>
#include <string.h>
#include "handler.h"

static struct list topics = LIST_HEAD_INIT(topics);

int topic_register(const char *name) {
    struct topic *top = topic_get(name);
    if (top) return -1;
    struct topic *new_top = malloc(sizeof (struct topic));
    new_top->name = strdup(name);
    new_top->count = 0;
    new_top->created_time = new_top->last_active = 0;
    LIST_ADDQ(&topics, &new_top->list);
    return 0;
}

int topic_unregister(const char *name) {
    struct topic *top = topic_get(name);
    if (!top) return -1;
    LIST_DEL(&top->list);
    free(top);
    return 0;
}

int topic_join(const char *name, int fd) {
    struct topic *top = topic_get(name);
    if (!top) return -1;
    return 0;
}

int topic_left(const char *name, int fd) {
    return 0;
}

struct topic* topic_get(const char* name) {
    struct topic *top;

    list_for_each_entry(top, &topics, list) {
        if (!strcmp(top->name, name))
            return top;
    }
    return 0;
}