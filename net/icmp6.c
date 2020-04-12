#include <netinet/in.h>
#include <net.h>
#include <string.h>

#define ICMP6_TYPE_ECHO_REQUEST 128
#define ICMP6_TYPE_ECHO_REPLY 129
#define ICMP6_TYPE_NEIGHBOR_SOLICITATION 135
#define ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT 136

#define ICMP6_FLAG_OVERRIDE 0x20
#define ICMP6_FLAG_SOLICITED 0x40

#define ICMP6_OPT_TYPE_SOURCE_LL_ADDR 1
#define ICMP6_OPT_TYPE_TARGET_LL_ADDR 2

#define ICMP6_MAX_LENGTH (ETH_MTU - ETH_HLEN - IP6_HEADER_LEN)

static efi_status net_icmp6_send(const void* data, size_t length, const struct in6_addr *daddr) {
	ip6_packet_t* p = net_bufpool_get(ETH_MTU);

	if(!p) {
		return EFI_ABORTED;
	}

	struct icmp6_header *icmp = (void *) p->data;

	if(length > ICMP6_MAX_LENGTH) {
		efi_printf("Internal error: ICMP write request is too long\n");
		goto fail;
	}

	if(net_ip6_packet_setup(p, daddr, length, IPPROTO_ICMP6)) {
		efi_printf("Error: ip6_setup failed!\n");
		goto fail;
	}

	memcpy(icmp, data, length);
	icmp->checksum = (uint16_t) net_ip6_checksum(&p->ip6, IPPROTO_ICMP6, length);

	return net_eth_send(p, ETH_HLEN + IP6_HEADER_LEN + length);

fail:
	net_bufpool_return(p);
	return EFI_ABORTED;
}

void net_icmp6_receive(ip6_header_t *ip, void *data, size_t len) {
	struct icmp6_header *icmp = data;

	if(icmp->type == ICMP6_TYPE_ECHO_REQUEST) {
		icmp->checksum = 0;
		icmp->type = ICMP6_TYPE_ECHO_REPLY;

		net_icmp6_send(data, len, (void *) &ip->src);
	} else if(icmp->type == ICMP6_TYPE_NEIGHBOR_SOLICITATION) {
		struct icmp6_ndp *ndp = data;

		/* validate the message as specified in section 7.1.1 of RFC 4861 */
		if(len < 24 || ndp->code != 0) {
			return; /* the RFC requires invalid messages to be silently discarded */
		}

		if(ip->hop_limit != 255 || memcmp(ndp->target, &ip6_ll_addr, 16)) {
			return;
		}
		/* construct a reply */
		struct {
			struct icmp6_ndp header;
			uint8_t options[8];
		} reply;

		reply.header.type = ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT;
		reply.header.code = 0;
		reply.header.checksum = 0;
		reply.header.flags = ICMP6_FLAG_SOLICITED | ICMP6_FLAG_OVERRIDE;
		memcpy(reply.header.target, &ip6_ll_addr, IP6_ADDR_LEN);

		reply.options[0] = ICMP6_OPT_TYPE_TARGET_LL_ADDR;
		reply.options[1] = 1; /* length in units of 8 bytes */
		memcpy(&reply.options[2], &ll_mac_addr, ETH_ALEN);

		net_icmp6_send(&reply, sizeof(reply), (void *) &ip->src);
	}
}
