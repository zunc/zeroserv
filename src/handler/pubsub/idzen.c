#include "idzen.h"
#include "common/uthash.h"

struct id_item {
    char *name;
    long inc_id;
    UT_hash_handle hh;
};

struct id_item *id_list = NULL;

struct id_item* id_get(const char *name) {
    struct id_item *id = NULL;
    HASH_FIND_STR(id_list, name, id);
    return id;
}

long id_zen(const char *name) {
    struct id_item *id = id_get(name);
    if (!id) {
        //unsigned int id_count = HASH_COUNT(id_list);
        id = (struct id_item*) malloc(sizeof(struct id_item));
        id->name = strdup(name);
        id->inc_id = 0;
        HASH_ADD_STR(id_list, name, id);
    }
    return ++id->inc_id;
}

