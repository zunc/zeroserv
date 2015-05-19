#include <stdlib.h>
#include <string.h>
#include "handler.h"
#include "common.h"

static struct list topics = LIST_HEAD_INIT(topics);

struct topic* topic_create(const char *name) {
    struct topic *top = topic_get(name);
    if (top) return NULL;
    struct topic *new_top = malloc(sizeof (struct topic));
    new_top->name = strdup(name);
    new_top->count = 0;
    new_top->created_time = new_top->last_active = 0;
    new_top->members = vector_create(20, 2);
    LIST_ADDQ(&topics, &new_top->list);
    return new_top;
}

int topic_delete(const char *name) {
    struct topic *top = topic_get(name);
    if (!top) return -1;
    int i;
    for (i = 0; i < top->members.size; i++) {
        struct account *acc = vector_access(&top->members, i);
        ASSERT(acc);
        topic_left(top, acc);
    }
    vector_destroy(&top->members);
    LIST_DEL(&top->list);
    free(top);
    return 0;
}

int topic_join(struct topic *top, struct account *acc) {
    vector_insert(&top->members, acc);
    vector_insert(&acc->sub_lists, top);
    return 0;
}

int topic_left(struct topic *top, struct account *acc) {
    vector_remove(&top->members, acc);
    vector_remove(&acc->sub_lists, top);
    return 0;
}

struct topic* topic_get(const char *name) {
    struct topic *top;

    list_for_each_entry(top, &topics, list) {
        if (!strcmp(top->name, name))
            return top;
    }
    return 0;
}