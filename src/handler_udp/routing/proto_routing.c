#include "proto_routing.h"
#include <arpa/inet.h>
#include "../../dbg.h"
#include "../../common.h"
#include "../../fd.h"
#include "../../buffer.h"
#include "route_map.h"
#include "../routing.h"
#include "../../util.h"
#include "../../common/netutil.h"
#include "mineip.h"
#include "local_port.h"

struct router_setting router_setting;
struct router_stat router_stat;

struct hash_task *tasks = NULL;
int fd_ = 0;

static int send_packet_direct(int fd, uint32_t host, uint16_t port, union packet *pk) {
	ASSERT(pk);
	struct sockaddr_in saddr;
	int slen = sizeof (struct sockaddr_in);
	memset((char *) &saddr, 0, slen);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = host;
	return sendto(fd, pk->context, pk->field.frameSize, 0, (struct sockaddr *) &saddr, slen);
}

static int send_packet(int fd, struct route* rout, union packet *pk) {
	ASSERT(rout);
	ASSERT(pk);
	int slen = sizeof (struct sockaddr_in);
	return sendto(fd, pk->context, pk->field.frameSize, 0, (struct sockaddr *) &rout->s_remote, slen);
}

#define MAX_HASH 128

static int zen_hash(union packet *pk, char *hash, int max_len) {
	// <!> consider optimal hash key
	// hash zen: src + dst + seq
	ASSERT(hash);
	ASSERT(pk);
	char src[20];
	char dst[20];
	ntoip(src, pk->field.srcIp);
	ntoip(dst, pk->field.dstIp);
	snprintf(hash, max_len, "%d-%s:%d-%s:%d", pk->field.seq,
			src, pk->field.srcPort, dst, pk->field.dstIp);
	return (strlen(hash) + 1);
}

static struct task * retry(struct task *t) {
	//	log_info("task: retry");
	ASSERT(t);

	struct packet_trip *trip = (struct packet_trip*) t->context;
	if (trip->entries++ >= router_setting.retry) {
		// In the end - Linkin Park :'[

		struct hash_task *ht;
		char hash[MAX_HASH];
		int n = zen_hash(trip->p, hash, MAX_HASH);
		HASH_FIND_STR(tasks, hash, ht);
		if (ht) {
			// remove item in hash table
			HASH_DEL(tasks, ht);
			FREE(ht->unique); // free hash key
			FREE(ht);
		}
		
		trip->cb(PST_RETRY_OVER, trip);
		
		// warning packet dropped
		//
		log_info("drop packet(%d)", trip->entries);
		router_stat.drop++;
		FREE(t);
		return NULL;
	} else {
		log_info("entries(%d)", trip->entries);
		router_stat.retries++;
		send_packet(trip->fd, trip->r, trip->p);
		t->expire = tick_add(now_ms, router_setting.timeout);
	}
	return t;
}

static int ack_response(int fd, union packet *pk) {
	char src[20];
	ntoip(src, pk->field.srcIp);
	char dest_detail[64];
	snprintf(dest_detail, sizeof (dest_detail) - 1, "%s:%d", src, pk->field.srcPort);

	struct route* rout = route_get_by_dst(dest_detail);
	if (!rout) {
		log_warn("\t [!] ack. source not match on rule routing");
		return 1;
	}

	log_info("\t -> ack_node: %s\n", rout->remote);
	union packet ack_pk;
	ack_pk.field.frameSize = HEADER_SIZE;
	ack_pk.field.srcIp = pk->field.dstIp;
	ack_pk.field.srcPort = pk->field.dstPort;
	ack_pk.field.dstIp = pk->field.srcIp;
	ack_pk.field.dstPort = pk->field.srcPort;
	ack_pk.field.seq = pk->field.seq;
	ack_pk.field.ack = pk->field.ack;
	ack_pk.field.control = CTL_ACK;
	ack_pk.field.checksum = 0;

	if (strcmp(rout->remote, "*")) {
		// rout next node
		send_packet(fd, rout, &ack_pk);
	} else {
		// direct to dest
		send_packet_direct(fd, ack_pk.field.dstIp, ack_pk.field.dstPort, &ack_pk);
	}
	return 0;
}

static int proc_mine(int fd, union packet *pk) {
	switch (pk->field.control) {
		case CTL_SEND:
		{
			log_info("RECV: packet(%s)", pk->field.data);
			if (ack_response(fd, pk)) {
				log_warn("ack. fail");
			}
			break;
		}
		case CTL_ACK:
		{
			// success transfer data to next node
			// remove task schedule
			// notify ack to sender ?
			struct hash_task *ht;
			char hash[MAX_HASH];
			int n = zen_hash(pk, hash, MAX_HASH);
			HASH_FIND_STR(tasks, hash, ht);
			if (!ht) {
				log_warn("ack packet(%d) meaningless", pk->field.seq);
			} else {
				// remove task
				log_info("ack recv(%d)", pk->field.seq);
				struct task *t = ht->task;
				struct packet_trip *trip = (struct packet_trip*) t->context;
				trip->cb(PST_SUCCESS, trip);
//				FREE(trip->p);
//				FREE(trip);
				task_delete(t);
				task_free(t);

				// remove item in hash table
				HASH_DEL(tasks, ht);
				FREE(ht->unique); // free hash key
				FREE(ht);

				// callback notify success sent a packet
				// 
			}
			break;
		}
	}
}

// oneway send packet

int routing_proc(int fd, union packet *pk) {
	char src[20];
	char dst[20];
	ntoip(src, pk->field.srcIp);
	ntoip(dst, pk->field.dstIp);

	//	printf(" - frameSize(%d), seq(%d), ack(%d), control(%d), checksum(0x%x)\n\t-src(%s:%d) -> dst(%s:%d)\n",
	//			pk->frameSize, pk->seq, pk->ack, pk->control, pk->checksum,
	//			src, pk->srcPort, dst, pk->dstPort);

	//--- verify target packet is me ?
	// check mine_ip and dst_port
	if (is_mine_ip(pk->field.dstIp)) {
		// check dest port
		struct local_port *port = get_local_port(pk->field.dstPort);
		if (port) {
			// packet for me
			proc_mine(fd, pk);
			return;
		}
	}

	//--- transfer data to next node
	char dest_detail[64];
	snprintf(dest_detail, sizeof (dest_detail) - 1, "%s:%d", dst, pk->field.dstPort);
	struct route* rout = route_get_by_dst(dest_detail);
	if (!rout) {
		log_warn("\t [!] source not match on rule routing");
		// warn stuff
		//
		return 1;
	}
	log_info("\t -> next_node: %s : %s", rout->remote, pk->field.data);

	// make a trip
	int len = pk->field.frameSize;
	// First time - Ronan Keating :*
	if (strcmp(rout->remote, "*")) {
		// rout next node
		send_packet(fd, rout, pk);
	} else {
		// direct to dest
		send_packet_direct(fd, pk->field.dstIp, pk->field.dstPort, pk);
	}
	return 0;
}

int routing_parse(int fd) {
	if (fd_)
		fd_ = fd;

	struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
	char *pos = ib->curr;
	int len = buffer_remain_read(ib);

	router_stat.recv++;
	if (len < HEADER_SIZE) {
		log_info("broken packet");
		return -1;
	}

	union packet *pk = malloc(len);
	memcpy(pk->context, pos, len);
	if (pk->field.frameSize > router_setting.max_packet) {
		log_warn("frameSize large than max_packet: %d", pk->field.frameSize);
		router_stat.broken++;
		FREE(pk);
		return 1;
	}

	if (pk->field.frameSize != len) {
		log_warn("recv not enough a packet: frameSize(%d), packet(%d)",
				pk->field.frameSize, len);
		router_stat.broken++;
		FREE(pk);
		return 1;
	}

	// checksum
	int data_len = len - HEADER_SIZE;
	unsigned short checkSum;
	if ((checkSum = tcp_checksum(pk->field.data, data_len)) != pk->field.checksum) {
		log_warn("incorrect checksum: pk.checkSum(0x%x), checkSum(0x%x)",
				pk->field.checksum, checkSum);
		router_stat.broken++;
		FREE(pk);
		return 1;
	}

	return routing_proc(fd, pk);
}