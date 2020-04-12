#pragma once

#include <uefi.h>

#define NETBOOT_MSG_LEN(x) (offsetof(vy_netboot_message_t, data) + (x))
#define NETBOOT_ADVERTISEMENT_LEN NETBOOT_MSG_LEN(0)
#define NETBOOT_ACK_LEN NETBOOT_MSG_LEN(sizeof(vy_netboot_ack_t))

#define NETBOOT_MSG_MAX_LEN 1452UL

/* link-local addresses */
extern struct ether_addr ll_mac_addr;
extern struct in6_addr ip6_ll_addr;

/* solicited-node multicast addresses */
extern struct ether_addr snm_mac_addr;
extern struct in6_addr ip6_snm_addr;

/* cache for the remote addresses */
extern struct ether_addr remote_mac_addr;
extern struct in6_addr remote_ip6_addr;

/* netboot state */
extern bool net_poll_continue;
extern bool net_stop_advertising;

typedef enum {
	CMD_ADVERTISE,
	CMD_STOP_ADVERTISING,
	CMD_SHUTDOWN,
	CMD_FWSETUP,
	CMD_REBOOT,
	CMD_FILE_SEND_INIT,
	CMD_FILE_FRAGMENT,
	CMD_ACK,
	CMD_BOOT,
} vy_netboot_cmd_t;

typedef struct {
	vy_netboot_cmd_t cmd;
	uint64_t message_length;
	uint8_t data[0];
} vy_netboot_message_t;

typedef struct {
	size_t file_size;
	size_t data_per_packet;
	size_t packets;
	size_t path_len;
	char path[0];
} vy_netboot_file_info_t;

typedef struct {
	size_t fragment_id;
	size_t data_len;
	uint8_t data[0];
} vy_netboot_file_fragment_t;

typedef struct {
	vy_netboot_cmd_t type;
	size_t file_fragment;
} vy_netboot_ack_t;

typedef enum {
	STATE_PREINIT,
	STATE_ADVERTISING,
	STATE_CMD_WAIT,
	STATE_FILE_TRANSFER,
} vy_netboot_state_t;

typedef struct {
	vy_netboot_state_t state;

	vy_netboot_file_info_t *file_info;
	uint8_t *file_buffer;
	size_t file_bytes_transferred;
	size_t file_packets_transferred;
} vy_netboot_t;

vy_netboot_t *netboot_get_state(void);
void netboot_cmd_send(vy_netboot_cmd_t cmd, void *data, size_t data_len);
efi_status netboot_cmd_receive(void *data);
