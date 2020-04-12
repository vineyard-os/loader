#pragma once

#include <stdint.h>

struct ethbuf {
	struct ethbuf *next;
	uint8_t data[0];
};

#define ETH_BUFFERS 4
#define ETH_BUFFERS_PAGES ((ETH_BUFFERS >> 1) + (ETH_BUFFERS % 2))

void net_bufpool_init(void);
void net_bufpool_return(void *data);
void *net_bufpool_get(size_t size);
