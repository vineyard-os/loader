#include <arpa/inet.h>
#include <netinet/in.h>
#include <net.h>
#include <string.h>

const struct in6_addr ip6_all_nodes = {
    .s6_addr = {0xFF, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
};

/* our current link-local addresses */
struct ether_addr ll_mac_addr;
struct in6_addr ip6_ll_addr;

/* solicited-node multicast addresses */
struct ether_addr snm_mac_addr;
struct in6_addr ip6_snm_addr;

/* remote address cache */
struct ether_addr remote_mac_addr;
struct in6_addr remote_ip6_addr;

bool net_ip6_empty(struct in6_addr *ip) {
	for(size_t i = 0; i < IP6_ADDR_LEN; i++) {
		if(ip->s6_addr[i]) {
			return false;
		}
	}

	return true;
}

static uint16_t checksum(const void *data, size_t len, uint16_t sum_init) {
	uint32_t sum = sum_init;
	const uint16_t *current_data = data;

	/* add up 16 bits at a time */
	while(len > 1) {
		sum += *current_data++;
		len -= 2;
	}

	/* add odd bytes */
	if(len) {
		sum += (*current_data & 0xFF);
	}

	/* fold the sum to 16 bits */
	while(sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return (uint16_t) sum;
}

uint16_t net_ip6_checksum(ip6_header_t *ip, uint16_t type, size_t length) {
	uint16_t sum;

	sum = checksum(&ip->length, sizeof(ip->length), htons(type));
	sum = checksum(&ip->src, 32 + length, sum);

	return ~sum;
}

static void net_ip6_multicast_from_ip6(struct ether_addr *mac, const struct in6_addr *ip) {
	mac->ether_addr_octet[0] = 0x33;
	mac->ether_addr_octet[1] = 0x33;
	mac->ether_addr_octet[2] = ip->s6_addr[12];
	mac->ether_addr_octet[3] = ip->s6_addr[13];
	mac->ether_addr_octet[4] = ip->s6_addr[14];
	mac->ether_addr_octet[5] = ip->s6_addr[15];
}

static void net_ip6_ll_addr_from_mac(struct in6_addr *ip, const struct ether_addr *mac) {
	memset(ip + 2, 0, 6);

	ip->s6_addr[0] = 0xFE;
	ip->s6_addr[1] = 0x80;
	ip->s6_addr[8] = mac->ether_addr_octet[0] ^ 2;
	ip->s6_addr[9] = mac->ether_addr_octet[1];
	ip->s6_addr[10] = mac->ether_addr_octet[2];
	ip->s6_addr[11] = 0xFF;
	ip->s6_addr[12] = 0xFE;
	ip->s6_addr[13] = mac->ether_addr_octet[3];
	ip->s6_addr[14] = mac->ether_addr_octet[4];
	ip->s6_addr[15] = mac->ether_addr_octet[5];
}

static void net_ip6_snm_addr_from_mac(struct in6_addr *ip, const struct ether_addr *mac) {
	memset(&ip->s6_addr[2], 0, 9);

	ip->s6_addr[0] = 0xFF;
	ip->s6_addr[1] = 0x02;
	ip->s6_addr[11] = 0x01;
	ip->s6_addr[12] = 0xFF;
	ip->s6_addr[13] = mac->ether_addr_octet[3];
	ip->s6_addr[14] = mac->ether_addr_octet[4];
	ip->s6_addr[15] = mac->ether_addr_octet[5];
}

static efi_status net_ip6_resolve(struct ether_addr *mac, const struct in6_addr *ip) {
	if(ip->s6_addr[0] == 0xFF) {
		net_ip6_multicast_from_ip6(mac, ip);
		return EFI_SUCCESS;
	}

	if(!memcmp(ip, &remote_ip6_addr, sizeof(remote_ip6_addr))) {
		memcpy(mac, &remote_mac_addr, sizeof(remote_mac_addr));

		return EFI_SUCCESS;
	}

	return EFI_NOT_FOUND;
}

efi_status net_ip6_packet_setup(ip6_packet_t *p, const struct in6_addr *addr, size_t length, uint8_t type) {
	struct ether_addr mac;

	if(net_ip6_resolve(&mac, addr)) {
		return EFI_NOT_FOUND;
	}

	/* ethernet header setup */
	memcpy(&p->eth.h_dest, &mac, ETH_ALEN);
	memcpy(&p->eth.h_source, &ll_mac_addr, ETH_ALEN);
	p->eth.h_protocol = htons(ETH_IP6);

	/* IPv6 header setup */
	p->ip6.ver_tc_flow = 0x60;
	p->ip6.length = htons((uint16_t) length);
	p->ip6.next_header = type;
	p->ip6.hop_limit = 255;
	memcpy(&p->ip6.src, &ip6_ll_addr, sizeof(struct in6_addr));
	memcpy(&p->ip6.dst, addr, sizeof(struct in6_addr));

	return EFI_SUCCESS;
}

void net_ip6_init(struct ether_addr *mac) {
	memcpy(&ll_mac_addr, mac, sizeof(struct ether_addr));
	struct ether_addr all;

	net_ip6_ll_addr_from_mac(&ip6_ll_addr, &ll_mac_addr);
	net_ip6_snm_addr_from_mac(&ip6_snm_addr, &ll_mac_addr);

	net_ip6_multicast_from_ip6(&snm_mac_addr, &ip6_snm_addr);
	net_ip6_multicast_from_ip6(&all, &ip6_all_nodes);

	net_add_mcast_filter(&snm_mac_addr);
	net_add_mcast_filter(&all);
}
