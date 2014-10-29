#ifndef __dbg_h__
#define __dbg_h__
// ref: http://c.learncodethehardway.org/book/ex20.html
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "config.h"

char* get_formatted_time(void);

#ifdef LOG_COLOURED
#define GRAY(s)   "\033[1;30m" s "\033[0m"
#define RED(s)    "\033[0;31m" s "\033[0m"
#define GREEN(s)  "\033[0;32m" s "\033[0m"
#define YELLOW(s) "\033[1;33m" s "\033[0m"
#else
#define GRAY(s)   s
#define RED(s)    s
#define GREEN(s)  s
#define YELLOW(s) s
#endif

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define log(format, loglevel, ...) printf("%s %s " format "\n", get_formatted_time(), loglevel, ## __VA_ARGS__)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(format, ...) log(format, GRAY("ERR"), ## __VA_ARGS__)
#define log_fatal(format, ...) log(format, RED("FATAL"), ## __VA_ARGS__); exit(1)
#define log_warn(format, ...) log(format, YELLOW("WARN"), ## __VA_ARGS__)
#define log_info(format, ...) log(format, GREEN("INFO"), ## __VA_ARGS__)

#define log_dbg(format, ...) if (DEBUG) printf("[%s:%d] " format "\n", __FILE__, __LINE__, ## __VA_ARGS__)

void log_pid(const char *msg);

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif