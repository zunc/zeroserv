#include "model.h"
#include "type/fd.h"
#include <stdio.h>

int model_acc_create(const int fd, const char* acc, const char* auth) {
    printf("[CREATE] user(\"%s\"), pass(\"%s\")\n", acc, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (account_create(acc, auth) < 0) {
        sprintf(ob->curr, "create user error\n");
    } else {
        sprintf(ob->curr, "create user(\"%s\") success\n", acc);
    }
    ob->r += strlen(ob->curr);
    return ZB_SET_WR;
}

int model_acc_auth(const int fd, const char* acc, const char* auth) {
    printf("[LOGIN] user(\"%s\"), pass(\"%s\")\n", acc, auth);
    struct buffer *ob = fdtab[fd].cb[DIR_WR].b;
    if (account_auth(acc, auth)) {
        sprintf(ob->curr, "auth user error\n");
    } else {
        sprintf(ob->curr, "auth user(\"%s\") success\n", acc);
    }
    ob->r += strlen(ob->curr);
    return ZB_SET_WR;
}