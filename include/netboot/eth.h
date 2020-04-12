#pragma once

#include <net/ethernet.h>
#include <stdint.h>
#include <uefi.h>

#define ETH_ALEN 6
#define ETH_HLEN 14
#define ETH_MTU 1514

#define ETH_IP4 0x0800
#define ETH_ARP 0x0806
#define ETH_IP6 0x86DD

struct ethhdr {
	uint8_t h_dest[ETH_ALEN];
	uint8_t h_source[ETH_ALEN];
	uint16_t h_protocol;
};

extern uint32_t eth_bufpool_avail;
extern efi_physical_addr eth_buffer_phys;
extern efi_simple_network_protocol *snp;

efi_status net_eth_send(void *data, size_t len);
efi_status net_eth_receive(void *data, size_t len);
efi_status net_add_mcast_filter(const struct ether_addr *addr);
