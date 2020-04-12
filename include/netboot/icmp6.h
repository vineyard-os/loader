#pragma once

struct icmp6_header {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
} __attribute__((packed));

struct icmp6_ndp {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t flags;
	uint8_t target[IP6_ADDR_LEN];
	uint8_t options[0];
} __attribute__((packed));

void net_icmp6_receive(ip6_header_t *ip, void *data, size_t len);
