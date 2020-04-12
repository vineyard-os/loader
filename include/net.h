#pragma once

#include <netboot/bufpool.h>
#include <netboot/eth.h>
#include <netboot/ip6.h>
#include <netboot/icmp6.h>
#include <netboot/udp6.h>

#include <netboot/netboot.h>

void net_kernel_load(void);
