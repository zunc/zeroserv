/* 
 * File:   signal.h
 * Author: khoai
 *
 * Created on August 19, 2014, 5:31 PM
 */

#ifndef SIGNAL_H
#define	SIGNAL_H

#include <signal.h>
#include "type/signal.h"

extern int signal_queue_len;
extern struct signal_descriptor signal_state[];

void signal_init();
void signal_handler(int sig);
void signal_register(int sig, void (*handler)(int));
void __signal_process_queue();

static inline void signal_process_queue() {
    if (signal_queue_len > 0)
        __signal_process_queue();
}

#endif	/* SIGNAL_H */

