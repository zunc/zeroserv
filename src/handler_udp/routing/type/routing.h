/* 
 * File:   routing.h
 * Author: khoai
 *
 * Created on October 22, 2014, 10:27 AM
 */

#ifndef ROUTING_H
#define	ROUTING_H

#include "../../..//task.h"
#include "../../../common/time.h"
#include "../../../common/uthash.h"

#define HEADER_SIZE 24

enum CONTROL {
    CTL_SEND = 1,
    CTL_ACK = 2
};

enum PACKET_STATE {
    PST_SUCCESS = 0,
    PST_RETRY_OVER = 1
};

#pragma pack(push)
#pragma pack(2)

union packet {

    struct {
        uint32_t frameSize;
        uint32_t srcIp; // src
        uint16_t srcPort;
        uint32_t dstIp; // dest
        uint16_t dstPort;
        uint32_t seq;
        uint8_t ack;
        uint8_t control;
        uint16_t checksum;

        char data[0];
    } field;
    char context[0];
};
#pragma pack(pop)

typedef int (*cb_ack)(int result, void* context);

struct packet_trip {
    int fd;
    struct route *r;
    union packet *p;
    int entries;
    unsigned int time_start;
    cb_ack cb;
};

// wrapper struct task for hashtable, just don't change origin struct

struct hash_task {
    //	int seq; // sequence id
    char* unique;
    struct task *task; // task node
    UT_hash_handle hh; // hashtable
};

#endif	/* ROUTING_H */

