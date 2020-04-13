#include <net.h>
#include <uefi.h>
#include <string.h>

static efi_mac_addr mcast_filters[8];
static unsigned mcast_filter_count = 0;

efi_simple_network_protocol *snp;
static efi_event net_timer = NULL;

efi_physical_addr eth_buffer_phys = 0;

bool net_poll_continue = true;
static bool net_advertised = false;
bool net_stop_advertising = false;

static void net_timer_set(uint32_t ms) {
	if(!net_timer) {
		return;
	}

	efi_status status = st->BootServices->SetTimer(net_timer, TimerRelative, (uint64_t) ms * 10000UL);
	EFIERR(status);
}

static bool net_timer_expired(void) {
	if(!net_timer) {
		return false;
	}

	if(st->BootServices->CheckEvent(net_timer) == EFI_SUCCESS) {
		return true;
	}

	return false;
}

static void net_advertise(void) {
	netboot_cmd_send(CMD_ADVERTISE, NULL, 0);
}

static efi_status net_get_protocol(void) {
	efi_simple_network_protocol *snp_current;
	/* TODO: don't limit this to 32 handles */
	efi_handle handles[32];
	char16_t *paths[32];
	size_t nic_count = 0;
	size_t size = sizeof(handles);
	size_t previous_device = 0;
	uint32_t interrupt_status;
	void *tx_buf;

	efi_status status = st->BootServices->LocateHandle(ByProtocol, &SimpleNetworkProtocol, NULL, &size, handles);
	EFIERR(status);

	nic_count = size / sizeof(efi_handle);

	for(size_t i = 0; i < nic_count; i++) {
		paths[i] = uefi_handle_get_string(handles[i]);
	}

	for(size_t i = 0; i < nic_count; i++) {
		/* Avoid trying to handle the same device more than once */
		if(i != previous_device) {
			if(!memcmp(paths[i], paths[previous_device], efi_strlen(paths[previous_device]))) {
				continue;
			} else {
				previous_device = i;
			}
		}

		st->ConOut->OutputString(st->ConOut, paths[i]);

		status = st->BootServices->HandleProtocol(handles[i], &SimpleNetworkProtocol, (void **) &snp_current);
		if(EFI_ERROR(status)) {
			efi_printf(": failed to open (%zu)\n", status);
			goto fail;
		}

		status = snp_current->Start(snp_current);
		if(EFI_ERROR(status) && status != EFI_ALREADY_STARTED) {
			efi_printf(": failed to start (%zu)\n", status);
			goto fail;
		}

		if(status != EFI_ALREADY_STARTED) {
			status = snp_current->Initialize(snp_current, 0, 0);

			if(EFI_ERROR(status)) {
				efi_printf(": failed to initialize (%zu)\n", status);
				goto fail;
			}
		}

		status = snp_current->GetStatus(snp_current, &interrupt_status, &tx_buf);
		if(EFI_ERROR(status)) {
			efi_printf(": failed to read status (%zu)\n", status);
			goto fail;
		}

		if(!snp_current->Mode->MediaPresent) {
			efi_printf(": no link detected\n");
			goto fail;
		}

		efi_printf(": link detected\n");

		snp = snp_current;
		return EFI_SUCCESS;

	fail:
		st->BootServices->CloseProtocol(handles[i], &SimpleNetworkProtocol, handle, NULL);
		snp_current = NULL;
	}

	return EFI_NOT_FOUND;
}

efi_status net_add_mcast_filter(const struct ether_addr *addr) {
	if(mcast_filter_count >= 8 || mcast_filter_count >= snp->Mode->MaxMCastFilterCount) {
		return EFI_INVALID_PARAMETER;
	}

	memcpy(mcast_filters + mcast_filter_count, addr, ETH_ALEN);
	mcast_filter_count++;

	return EFI_SUCCESS;
}

static efi_status net_open(void) {
	efi_status status = st->BootServices->CreateEvent(EVT_TIMER, TPL_CALLBACK, NULL, NULL, &net_timer);
	EFIERR(status);

	status = net_get_protocol();
	EFIERR(status);

	net_bufpool_init();

	net_ip6_init((struct ether_addr *) snp->Mode->CurrentAddress.addr);

	status = snp->ReceiveFilters(snp, EFI_SIMPLE_NETWORK_RECEIVE_UNICAST | EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST, 0, false, mcast_filter_count, (void *) mcast_filters);
	EFIERR(status);

	return EFI_SUCCESS;
}

static void net_close(void) {
	st->BootServices->SetTimer(net_timer, TimerCancel, 0);
	st->BootServices->CloseEvent(net_timer);
	snp->Shutdown(snp);
	snp->Stop(snp);
}

static void net_poll(void) {
	uint8_t data[2048];
	efi_status status;
	size_t size_header;
	size_t size_body;
	uint32_t interrupt_status;
	void *tx_buf;

	if(snp && !net_advertised) {
		net_advertise();
		net_timer_set(2000);

		net_advertised = true;
	} else if(net_timer_expired() && !net_stop_advertising) {
		net_advertise();
		net_timer_set(2000);
	}

	if(eth_bufpool_avail < 16) {
		if((status = snp->GetStatus(snp, &interrupt_status, &tx_buf))) {
			efi_printf("GetStatus returned %zu\n", status);
			return;
		}

		/* return buffers to the pool from completed Transmit() operations */
		if(tx_buf) {
			efi_physical_addr buf_paddr = (efi_physical_addr) tx_buf;

			if((buf_paddr >= eth_buffer_phys) && (buf_paddr < (eth_buffer_phys + (ETH_BUFFERS_PAGES * 0x1000)))) {
				net_bufpool_return(tx_buf);
			} else {
				efi_printf("invalid buffer returned to pool\n");
			}
		}
	}

	size_header = 0;
	size_body = sizeof(data);

	status = snp->Receive(snp, &size_header, &size_body, data, NULL, NULL, NULL);

	if(status != EFI_SUCCESS) {
		if(status != EFI_NOT_READY) {
			EFIERR(status);
		}

		return;
	}

	status = net_eth_receive(data, size_body);
}

void net_kernel_load(void) {
	net_open();

	while(net_poll_continue) net_poll();

	net_close();
}
