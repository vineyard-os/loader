#include <arpa/inet.h>
#include <net.h>
#include <string.h>

efi_status net_eth_send(void *data, size_t len) {
	efi_status status = snp->Transmit(snp, 0, len, (void *) data, NULL, NULL, NULL);

	if(EFI_ERROR(status)) {
		net_bufpool_return(data);
		return EFI_DEVICE_ERROR;
	} else {
		return EFI_SUCCESS;
	}
}

efi_status net_eth_receive(void *data, size_t len) {
	ip6_packet_t *packet = data;
	uint32_t n;

	if(len < (ETH_HLEN + IP6_HEADER_LEN)) {
		efi_printf("invalid packet length\n");
		return EFI_ABORTED;
	}

	if(packet->eth.h_protocol != htons(ETH_IP6)) {
		return EFI_ABORTED;
	}

	len -= ETH_HLEN + IP6_HEADER_LEN;

	if((packet->ip6.ver_tc_flow & 0xF0) != 0x60) {
		efi_printf("unknown IPv6 version\n");
		return EFI_ABORTED;
	}

	n = ntohs(packet->ip6.length);
	if(n > len) {
		efi_printf("IPv6 length mismatch\n");
		return EFI_ABORTED;
	}

	len = n;

	if(memcmp(&ip6_ll_addr, &packet->ip6.dst, 16) && memcmp(&ip6_snm_addr, &packet->ip6.dst, 16) && memcmp(&ip6_all_nodes, &packet->ip6.dst, 16)) {
		return EFI_SUCCESS;
	}

	memcpy(&remote_mac_addr, &packet->eth.h_source, 6);
	memcpy(&remote_ip6_addr, &packet->ip6.src, 16);

	if(packet->ip6.next_header == 17) {
		net_udp6_receive(&packet->ip6, &packet->data, len);
	} else if(packet->ip6.next_header == 58) {
		net_icmp6_receive(&packet->ip6, &packet->data, len);
	}

	return EFI_SUCCESS;
}
