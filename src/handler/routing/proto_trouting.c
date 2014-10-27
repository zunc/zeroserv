#include "../../handler_udp/routing/proto_routing.h"
#include "proto_trouting.h"
////#include "../../buffer.h"
//#include "../../protos.h"
//#include "../../dbg.h"
//#include "../../common/mini-clist.h"
//#include "../../fd.h"
//#include "../../common.h"
//#include "../../handler.h"
//
//struct router_setting trouter_setting;
//struct router_stat trouter_stat;
//
//int trouting_parse(int fd) {
//	struct buffer *ib = fdtab[fd].cb[DIR_WR].b;
//	char *pos = ib->curr;
//	int len = buffer_remain_read(ib);
//
//	trouter_stat.recv++;
//	if (len < HEADER_SIZE) {
//		log_info("broken packet");
//		return -1;
//	}
//
//	struct packet pk;
//
//	// parse
//	pk.frameSize = *(uint32_t*) (pos);
//	pk.srcIp = *(uint32_t*) (pos + 4);
//	pk.srcPort = *(uint16_t*) (pos + 8);
//	pk.dstIp = *(uint32_t*) (pos + 10);
//	pk.dstPort = *(uint16_t*) (pos + 14);
//	pk.seq = *(uint32_t*) (pos + 16);
//	pk.ack = *(uint8_t*) (pos + 20);
//	pk.control = *(uint8_t*) (pos + 21);
//	// reverse
//	pk.checksum = *(uint16_t*) (pos + 22);
//	pk.data = pos + 24;
//
//	if (pk.frameSize > trouter_setting.max_packet) {
//		log_warn("frameSize large than max_packet: %d", pk.frameSize);
//		trouter_stat.broken++;
//		return 1;
//	}
//
//	if (pk.frameSize != len) {
//		log_warn("recv not enough a packet: frameSize(%d), packet(%d)",
//				pk.frameSize, len);
//		trouter_stat.broken++;
//		return 1;
//	}
//
//	// checksum
//	int data_len = pk.frameSize - HEADER_SIZE;
//	unsigned short checkSum;
//	if ((checkSum = tcp_checksum(pk.data, data_len)) != pk.checksum) {
//		log_warn("incorrect checksum: pk.checkSum(0x%x), checkSum(0x%x)",
//				pk.checksum, checkSum);
//		trouter_stat.broken++;
//		return 1;
//	}
//
////	return routing_proc(fd, &pk, pos);
//	return 0;
//}
//
