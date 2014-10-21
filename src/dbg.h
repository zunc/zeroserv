#ifndef __dbg_h__
#define __dbg_h__
// ref: http://c.learncodethehardway.org/book/ex20.html
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "config.h"

char* get_formatted_time(void);

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define log(format, loglevel, ...) printf("%s " format "\n", get_formatted_time(), ## __VA_ARGS__)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(format, ...) log(format, "ERR", ## __VA_ARGS__)
#define log_fatal(format, ...) log(format, "FATAL", ## __VA_ARGS__); exit(1)
#define log_warn(format, ...) log(format, "WARN", ## __VA_ARGS__)
#define log_info(format, ...) log(format, "INFO", ## __VA_ARGS__)

#define log_dbg(format, ...) if (DEBUG) printf("[%s:%d] " format "\n", __FILE__, __LINE__, ## __VA_ARGS__)

void log_pid(const char *msg);

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif