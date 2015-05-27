/* 
 * File:   cookie.h
 * Author: khoai
 *
 * Created on May 21, 2015, 2:10 PM
 */

#ifndef COOKIE_H
#define	COOKIE_H

#include <time.h>
#include <stdlib.h>
#include "common/uthash.h"

#define COOKIE_LEN 10
#define COOKIE_LEN_STR 11

static void init_rand() {
    srand(time(NULL));
}

static char* zen_cookie() {
    // zen a random cookie
    char *cookie = malloc(COOKIE_LEN + 1);
    char *pos = cookie;
    static span = 'z' - 'a';
    int i;
    for (i = 0; i < COOKIE_LEN; i++) {
        *pos = rand() % span + 'a';
        pos++;
    }
    *pos = 0;
    return cookie;
}

struct cookie_item {
    long time_save; // time save, use to remove expired cookie
    char name[COOKIE_LEN_STR]; // cookie name
    void *content; // user content
    UT_hash_handle hh;
};

static struct cookie_item *cookie_list = NULL;

static struct cookie_item *cookie_get(const char *cookie_name) {
    struct cookie_item *cookie = NULL;
    HASH_FIND_STR(cookie_list, cookie_name, cookie);
    return cookie;
}

static int cookie_remove(const char *name) {
    struct cookie_item *cookie = cookie_get(name);
    if (!cookie) return -1;
    HASH_DEL(cookie_list, cookie);
    return 0;
}

static void* cookie_get_content(const char *cookie_name) {
    struct cookie_item *cookie = cookie_get(cookie_name);
    if (!cookie) return NULL;
    return cookie->content;
}

static char* cookie_put_content(void *content) {
    struct cookie_item *cookie = malloc(sizeof(struct cookie_item));
    if (!cookie) return NULL;
    cookie->content = content;
    char *cookie_name = zen_cookie();
    strncpy(cookie->name, cookie_name, COOKIE_LEN);
    cookie->name[COOKIE_LEN] = 0;
    log_warn("[COOKIE] %s: 0x%lx", cookie_name, (long) content);
    free(cookie_name);
    HASH_ADD_STR(cookie_list, name, cookie);
    return cookie->name;
}

#endif	/* COOKIE_H */

