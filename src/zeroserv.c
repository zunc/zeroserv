/* 
 * File:   main.c
 * Author: khoai
 *
 * Created on April 3, 2014, 2:49 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "fd.h"
#include "protos.h"
#include "poller.h"
#include "global.h"
#include "dbg.h"
#include "zsignal.h"
#include "cfg.h"

#include "task.h"

const char cfg_default[] = "zero.cfg";
static int _is_stop = 0;

void dump(int sig) {
    log_info("dump");
}

void sig_soft_stop(int sig) {
    log_info("sig_soft_stop");
}

void sig_dump_state(int sig) {
    log_info("sig_dump_state");
}

void sig_int(int sig) {
    _is_stop = 1;
    log_info("[] SIG_INT");
}

void sig_term(int sig) {
    _is_stop = 1;
    log_info("[] SIG_TERM");
}

int env_init() {
    config_load(cfg_default);
    signal_init();

    // register signal
    signal_register(SIGQUIT, dump);
    signal_register(SIGUSR1, sig_soft_stop);
    signal_register(SIGHUP, sig_dump_state);
    signal_register(SIGINT, sig_int);
    signal_register(SIGTERM, sig_term);
    signal(SIGPIPE, SIG_IGN);

    tv_update_date(-1, -1);
    fd_init(global.maxconn);
    poll_init();
    return 0;
}

int env_deinit() {
    poll_term();
    protocol_unbind_all();
    log_info("server shutdown");
    return 0;
}

int proc_center() {
    int next;
    tv_update_date(0, 1);
    while (1) {
        signal_process_queue();
        wake_expired_tasks(&next); // check exprired tasks
        process_runnable_tasks(&next); // process task
        poll_do(next); // event-loop for socket event
        // check stop condition
        if (_is_stop) {
            break;
        }
    }
}

#include "handler/pubsub/idzen.h"
int main(int argc, char** argv) {
//    //--- test id_zen
//    int i;
//    int ngroup = 5;
//    for (i = 0; i < 100; i++) {
//        char group[24] = {};
//        sprintf(group, "group_%d", i % ngroup);
//        long id = id_zen(group);
//        printf("%s: %ld\n", group, id);
//    }
//    return 0;

    env_init();
    protocol_bind_all();
    log_info("init complete");

    // fork mode
    log_pid("main");
    if (global.nbproc > 1) {
        log_info("fork mode: nproc(%d)", global.nbproc);
        int i;
        int ret;
        for (i = 0; i < global.nbproc; i++) {
            ret = fork();
            if (ret < 0) {
                log_fatal("fork fail _ _!");
                protocol_unbind_all();
            } else if (ret == 0) // child
                break;
        }

        if (i == global.nbproc) {
            exit(0); // bye bye boss
        }
        poll_fork();
    }

    protocol_enable_all();
    proc_center();
    env_deinit();
    return (EXIT_SUCCESS);
}

