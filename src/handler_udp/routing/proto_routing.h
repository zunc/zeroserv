/* 
 * File:   proto_routing.h
 * Author: khoai
 *
 * Created on October 16, 2014, 11:38 AM
 */

#ifndef PROTO_ROUTING_H
#define	PROTO_ROUTING_H

#include <arpa/inet.h>
#include "../../dbg.h"
#include "route_map.h"
#include "../routing.h"
#include "../../task.h"
#include "../../common/time.h"
#include "../../common/uthash.h"
#include "../../util.h"
#include "mineip.h"
#include "local_port.h"

/*
 * frameSize :	4 byte		\
 * src:port :	4 + 2 byte	 |
 * dst:port :	4 + 2 byte	 |
 * seqId :		4 byte		 |	24
 * ack :		1 byte		 |
 * flag:		1 byte		 |
 * checksum:	2 byte		 | 
 //* sender_port: 2 byte	/
 * data :		n byte		
 */

struct router_setting router_setting;
struct router_stat router_stat;
#define HEADER_SIZE 24

enum CONTROL {
	CTL_SEND = 1,
	CTL_ACK = 2
};

struct packet {
	uint32_t frameSize;
	uint32_t srcIp, dstIp;
	uint16_t srcPort, dstPort;
	uint32_t seq;
	uint8_t ack, control;
	uint16_t checksum;
	char *data;
	char *context;
};

struct packet_trip {
	int fd;

	char *data;
	int len;
	struct route *r;

	int entries;
	unsigned int time_start;
};

// wrapper struct task for hashtable, just don't change origin struct

struct hash_task {
	//	int seq; // sequence id
	char* unique;
	struct task *task; // task node
	UT_hash_handle hh; // hashtable
};

struct hash_task *tasks = NULL;

static void ntoip(char *ip, uint32_t srcIp) {
	struct in_addr add_src;
	add_src.s_addr = srcIp;
	strcpy(ip, inet_ntoa(add_src));
}

static int send_packet_direct(int fd, uint32_t host, uint16_t port,
		unsigned const char *context, int len) {
	ASSERT(context);
	struct sockaddr_in saddr;
	int slen = sizeof (struct sockaddr_in);
	memset((char *) &saddr, 0, slen);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = host;
	return sendto(fd, context, len, 0, (struct sockaddr *) &saddr, slen);
}

static int send_packet(int fd, struct route* rout, const char *context, int len) {
	ASSERT(context);
	int slen = sizeof (struct sockaddr_in);
	return sendto(fd, context, len, 0, (struct sockaddr *) &rout->s_remote, slen);
}

static struct task * retry(struct task *t) {
	//	log_info("task: retry");
	ASSERT(t);

	struct packet_trip *trip = (struct packet_trip*) t->context;
	if (trip->entries++ >= router_setting.retry) {
		// In the end - Linkin Park :'[
		// warning packet dropped
		//
		log_info("drop packet(%d)", trip->entries);
		router_stat.drop++;
		FREE(trip->data);
		FREE(trip);
		FREE(t);
		return NULL;
	} else {
		log_info("entries(%d)", trip->entries);
		router_stat.retries++;
		send_packet(trip->fd, trip->r, trip->data, trip->len);
		t->expire = tick_add(now_ms, router_setting.timeout);
	}
	return t;
}

#define MAX_HASH 128

static int zen_hash(struct packet *pk, char *hash, int max_len) {
	// <!> consider optimal hash key
	// hash zen: src + dst + seq
	ASSERT(hash);
	ASSERT(pk);
	char src[20];
	char dst[20];
	ntoip(src, pk->srcIp);
	ntoip(dst, pk->dstIp);
	snprintf(hash, max_len, "%d-%s:%d-%s:%d", pk->seq, src, pk->srcPort, dst, pk->dstIp);
	return (strlen(hash) + 1);
}

static int zen_packet(struct packet *pk, char *data, int max_len) {
	ASSERT(pk);
	if (max_len < pk->frameSize) {
		log_warn("max_len < pk->frameSize");
		return 1;
	}
	uint32_t frameSize = pk->frameSize;
	uint32_t data_len = pk->frameSize - HEADER_SIZE;
	uint16_t checksum = tcp_checksum(pk->data, data_len);

	char *pos = data;

	*(uint32_t*) pos = frameSize;
	pos += 4;
	*(uint32_t*) pos = pk->srcIp;
	pos += 4;
	*(uint16_t*) pos = pk->srcPort;
	pos += 2;
	*(uint32_t*) pos = pk->dstIp;
	pos += 4;
	*(uint16_t*) pos = pk->dstPort;
	pos += 2;

	*(uint32_t*) pos = pk->seq;
	pos += 4;
	*(uint8_t*) pos = pk->ack;
	pos += 1;

	*(uint8_t*) pos = pk->control;
	pos += 1;

	*(uint16_t*) pos = checksum;
	pos += 2;

	memcpy(pos, (const char*) data, data_len);
	return 0;
}

static int ack(int fd, struct packet *pk) {
	char src[20];
	ntoip(src, pk->srcIp);
	char dest_detail[64];
	snprintf(dest_detail, sizeof (dest_detail) - 1, "%s:%d", src, pk->srcPort);
	
	struct route* rout = route_get_by_dst(dest_detail);
	if (!rout) {
		log_warn("\t [!] ack. source not match on rule routing");
		return 1;
	}

	log_info("\t -> ack_node: %s\n", rout->remote);
	struct packet ack_pk;
	ack_pk.frameSize = HEADER_SIZE;
	ack_pk.srcIp = pk->dstIp;
	ack_pk.srcPort = pk->dstPort;
	ack_pk.dstIp = pk->srcIp;
	ack_pk.dstPort = pk->srcPort;
	ack_pk.seq = pk->seq;
	ack_pk.ack = pk->ack;
	ack_pk.control = CTL_ACK;
	ack_pk.checksum = 0;
	ack_pk.data = NULL;

	char ack_packet[HEADER_SIZE];
	if (!zen_packet(&ack_pk, ack_packet, HEADER_SIZE)) {
		if (strcmp(rout->remote, "*")) {
			// rout next node
			send_packet(fd, rout, ack_packet, HEADER_SIZE);
		} else {
			// direct to dest
			send_packet_direct(fd, pk->srcIp, pk->srcPort, ack_packet, HEADER_SIZE);
		}
	}
	return 0;
}

static int proc_mine(int fd, struct packet *pk) {
	switch (pk->control) {
		case CTL_SEND:
		{
			log_info("RECV: packet(%s)", pk->data);
			if (ack(fd, pk)) {
				log_warn("ack. fail");
			}
			break;
		}
		case CTL_ACK:
			break;
	}
}

static int routing_proc(int fd, struct packet *pk, const char *context) {
	char src[20];
	char dst[20];
	ntoip(src, pk->srcIp);
	ntoip(dst, pk->dstIp);

//	printf(" - frameSize(%d), seq(%d), ack(%d), control(%d), checksum(0x%x)\n\t-src(%s:%d) -> dst(%s:%d)\n",
//			pk->frameSize, pk->seq, pk->ack, pk->control, pk->checksum,
//			src, pk->srcPort, dst, pk->dstPort);

	//--- verify target packet is me ?
	// check mine_ip and dst_port
	if (is_mine_ip(pk->dstIp)) {
		// check dest port
		struct local_port *port = get_local_port(pk->dstPort);
		if (port) {
			// packet for me
			proc_mine(fd, pk);
			return;
		}
	}

	//--- transfer data to next nodel
	char dest_detail[64];
	snprintf(dest_detail, sizeof (dest_detail) - 1, "%s:%d", dst, pk->dstPort);
	struct route* rout = route_get_by_dst(dest_detail);
	if (!rout) {
		log_warn("\t [!] source not match on rule routing");
		// warn stuff
		//
		return 1;
	}
	log_info("\t -> next_node: %s : %s", rout->remote, pk->data);

	// make a trip
	int len = pk->frameSize;
	// First time - Ronan Keating :*
	if (strcmp(rout->remote, "*")) {
		// rout next node
		send_packet(fd, rout, context, len);
	} else {
		// direct to dest
		send_packet_direct(fd, pk->dstIp, pk->dstPort, context, len);
	}
	return 0;
}

static int routing_parse(int fd) {
	struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
	char *pos = ib->curr;
	int len = buffer_remain_read(ib);

	router_stat.recv++;
	if (len < HEADER_SIZE) {
		log_info("broken packet");
		return -1;
	}

	struct packet pk;

	// parse
	pk.frameSize = *(uint32_t*) (pos);
	pk.srcIp = *(uint32_t*) (pos + 4);
	pk.srcPort = *(uint16_t*) (pos + 8);
	pk.dstIp = *(uint32_t*) (pos + 10);
	pk.dstPort = *(uint16_t*) (pos + 14);
	pk.seq = *(uint32_t*) (pos + 16);
	pk.ack = *(uint8_t*) (pos + 20);
	pk.control = *(uint8_t*) (pos + 21);
	// reverse
	pk.checksum = *(uint16_t*) (pos + 22);
	pk.data = pos + 24;

	if (pk.frameSize > router_setting.max_packet) {
		log_warn("frameSize large than max_packet: %d", pk.frameSize);
		router_stat.broken++;
		return 1;
	}

	if (pk.frameSize != len) {
		log_warn("recv not enough a packet: frameSize(%d), packet(%d)",
				pk.frameSize, len);
		router_stat.broken++;
		return 1;
	}

	// checksum
	int data_len = pk.frameSize - HEADER_SIZE;
	unsigned short checkSum;
	if ((checkSum = tcp_checksum(pk.data, data_len)) != pk.checksum) {
		log_warn("incorrect checksum: pk.checkSum(0x%x), checkSum(0x%x)",
				pk.checksum, checkSum);
		router_stat.broken++;
		return 1;
	}

	return routing_proc(fd, &pk, pos);
}

#endif	/* PROTO_ROUTING_H */

