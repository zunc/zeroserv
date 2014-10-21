/* 
 * File:   common.h
 * Author: khoai
 *
 * Created on February 20, 2014, 11:30 AM
 */

#ifndef COMMON_H
#define	COMMON_H

#include "dbg.h"

#define FREE(x)                                \
if (x) {					\
	free(x);				\
	x = NULL;				\
}

#define LOG(exp)                                \
printf("%s:%d	%s\n", __FILE__, __LINE__, exp);


#define ASSERT(x)                              \
if (!x) {					\
	log_fatal("[ASSERT] is null");			\
}

#define BUFFER_SIZE 1024
#define INVALID_SOCKET -1

#define SLEEP_USEC(x) usleep(x)

#define ATOMIC_INCREASE(x)	__sync_fetch_and_add(x, 1)
#define ATOMIC_DECREASE(x)	__sync_fetch_and_sub(x, 1)
#define ATOMIC_READ(x)		__sync_fetch_and_add(x, 0)

#endif	/* COMMON_H */

