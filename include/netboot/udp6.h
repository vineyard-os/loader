#pragma once

#define UDP_HEADER_LEN 8

typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
	uint8_t eth[ETH_HLEN];
	ip6_header_t ip6;
	udp_header_t udp;
	uint8_t data[0];
} udp_packet_t;

efi_status net_udp6_send(const void *data, size_t dlen, const struct in6_addr *daddr, uint16_t dport, uint16_t sport);
efi_status net_udp6_receive(ip6_header_t *ip, void *_data, size_t len);
