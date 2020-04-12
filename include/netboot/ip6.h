#pragma once

#define IP6_ADDR_LEN 16
#define IP6_HEADER_LEN 40

struct in6_addr {
	uint8_t s6_addr[IP6_ADDR_LEN];
} __attribute__((packed));

typedef struct {
	uint32_t ver_tc_flow;
	uint16_t length;
	uint8_t next_header;
	uint8_t hop_limit;
	struct in6_addr src;
	struct in6_addr dst;
} __attribute__((packed)) ip6_header_t;

typedef struct {
	struct ethhdr eth;
	ip6_header_t ip6;
	uint8_t data[0];
} __attribute__((packed)) ip6_packet_t;

extern const struct in6_addr ip6_all_nodes;

bool net_ip6_empty(struct in6_addr *ip);
uint16_t net_ip6_checksum(ip6_header_t *op, uint16_t type, size_t length);
efi_status net_ip6_packet_setup(ip6_packet_t *p, const struct in6_addr *daddr, size_t length, uint8_t type);
void net_ip6_init(struct ether_addr *mac);
