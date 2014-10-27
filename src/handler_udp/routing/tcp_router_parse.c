#include "proto_routing.h"
#include <arpa/inet.h>
#include "../../dbg.h"
#include "../../common.h"
#include "../../fd.h"
#include "../../buffer.h"
#include "route_map.h"
#include "../routing.h"
#include "../../util.h"
#include "local_port.h"

struct ep_detail {
	uint32_t ip;
	uint16_t port;
	int fd;
};

struct packet_session {
	union packet *pk; // packet
	uint32_t total; // total size of data
	uint32_t current; // current size sent
	uint32_t max_size; // max size of packet
};

struct router_setting trouter_setting;

int cb_ack_(int result, void *context) {
	// ack a packet
	struct packet_trip *trip = (struct packet_trip*) context;
	switch (result) {
		case PST_SUCCESS:
		{
			// success a packet
			log_info("ack: packet {seq(%d)}", trip->p->field.seq);
			FREE(trip->p);
			FREE(trip);
			break;
		}
		case PST_RETRY_OVER:
		{
			// log packet
			//
			FREE(trip->p);
			FREE(trip);
			break;
		}
	}
	return 0;
}

int router_send_packet(int fd, union packet *pk) {
	// make NAT table
	//
	// replace source by node detail
	//pk->srcIp =
	//pk->srcPort =
	// split packet
	//struct packet *npk = malloc(sizeof(struct packet));
	//struct packet_session *pksess = malloc(sizeof(struct packet_session));

//	pk->field.srcIp = ntops("127.0.0.1");
//	unsigned int srcIp = trouter_setting.layer;
	unsigned int ip = get_ip_nic(trouter_setting.layer);
	if (!ip) {
		log_warn("not found ip by NIC");
		// FREE(pk);
		return;
	}
	pk->field.srcIp = ip;
	pk->field.srcPort = 222;
	if (routing_proc_ack(pk, cb_ack_)) {
		// fail
		//
	}
	return 0;
}

int tcp_router_parse(int fd) {
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

	return router_send_packet(fd, pk);
}