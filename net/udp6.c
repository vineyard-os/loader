#include <arpa/inet.h>
#include <netinet/in.h>
#include <net.h>
#include <string.h>

efi_status net_udp6_send(const void *data, size_t len, const struct in6_addr *addr, uint16_t dest_port, uint16_t source_port) {
	size_t length = len + UDP_HEADER_LEN;
	udp_packet_t *p = net_bufpool_get(ETH_MTU);

	if(!p) {
		return EFI_ABORTED;
	}

	if(net_ip6_packet_setup((void *) p, addr, length, IPPROTO_UDP)) {
		efi_printf("net_ip6_packet_setup failed");
		goto fail;
	}

	p->udp.src_port = htons(source_port);
	p->udp.dest_port = htons(dest_port);
	p->udp.length = htons((uint16_t) length);
	p->udp.checksum = 0;
	memcpy(p->data, data, len);

	p->udp.checksum = (uint16_t) net_ip6_checksum(&p->ip6, IPPROTO_UDP, length);

	return net_eth_send(p->eth, ETH_HLEN + IP6_HEADER_LEN + length);

fail:
	net_bufpool_return(p);
	return EFI_ABORTED;
}

efi_status net_udp6_receive(ip6_header_t *ip, void *data, size_t len) {
	udp_header_t *udp = data;
	(void) ip;
	(void) len;

	if(ntohs(udp->dest_port) == 57777) {
		return netboot_cmd_receive((void *) ((uintptr_t) data + UDP_HEADER_LEN));
	}

	return EFI_SUCCESS;
}
