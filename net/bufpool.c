#include <net.h>

static struct ethbuf *eth_bufpool;
uint32_t eth_bufpool_avail;

void net_bufpool_init(void) {
	EFIERR(st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 8, &eth_buffer_phys));

	uint8_t *ptr = (void *) eth_buffer_phys;
	for(size_t i = 0; i < ETH_BUFFERS; ++i) {
		struct ethbuf *buf = (void *) ptr;
		net_bufpool_return(buf);
		ptr += 2048;
	}
}

void net_bufpool_return(void *data) {
	efi_physical_addr buf_paddr = (efi_physical_addr) data;

	if((buf_paddr < eth_buffer_phys) || (buf_paddr >= (eth_buffer_phys + (ETH_BUFFERS_PAGES * 0x1000)))) {
		efi_printf("attempt to use buffer outside of allocated range");
	}

	struct ethbuf *buf = (void *) (buf_paddr & (~2047UL));

	buf->next = eth_bufpool;

	eth_bufpool_avail++;
	eth_bufpool = buf;
}

void *net_bufpool_get(size_t size) {
	if(size > ETH_MTU) {
		return NULL;
	}

	if(!eth_bufpool) {
		return NULL;
	}

	struct ethbuf *buf = eth_bufpool;
	eth_bufpool = buf->next;
	buf->next = NULL;
	eth_bufpool_avail--;

	return buf->data;
}
