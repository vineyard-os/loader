#include <net.h>
#include <stdlib.h>
#include <string.h>

static vy_netboot_t _netboot_state = {};

vy_netboot_t *netboot_get_state(void) {
	return &_netboot_state;
}

void netboot_cmd_send(vy_netboot_cmd_t cmd, void *data, size_t data_len) {
	vy_netboot_message_t *msg = malloc(NETBOOT_MSG_LEN(data_len));

	msg->cmd = cmd;
	msg->message_length = NETBOOT_MSG_LEN(data_len);

	if(data) {
		memcpy(msg->data, data, data_len);
	}

	if(!net_ip6_empty(&remote_ip6_addr)) {
		net_udp6_send(msg, msg->message_length, &remote_ip6_addr, 57778, 57777);
	} else {
		net_udp6_send(msg, msg->message_length, &ip6_all_nodes, 57778, 57777);
	}

	free(msg);
}

efi_status netboot_cmd_receive(void *data) {
	vy_netboot_message_t *msg = data;
	vy_netboot_t *s = netboot_get_state();

	if(s->state == STATE_FILE_TRANSFER) {
		if(msg->cmd == CMD_FILE_FRAGMENT) {
			vy_netboot_file_fragment_t *fragment = (void *) &msg->data;

			if(s->file_info->data_per_packet < fragment->data_len) {
				efi_printf("unexpected fragment (%u) data length: %u instead of %u\n", fragment->fragment_id, fragment->data_len, s->file_info->data_per_packet);
			}

			vy_netboot_ack_t ack = { .type = CMD_FILE_FRAGMENT, .file_fragment = 0 };

			if(fragment->fragment_id != s->file_packets_transferred) {
				// efi_printf("[%zu] packet out of order, got %u\n", s->file_packets_transferred, fragment->fragment_id);

				ack.file_fragment = (fragment->fragment_id < s->file_packets_transferred) ? fragment->fragment_id : s->file_packets_transferred;
			} else {
				memcpy(&s->file_buffer[s->file_bytes_transferred], fragment->data, fragment->data_len);

				ack.file_fragment = s->file_packets_transferred;

				s->file_bytes_transferred += fragment->data_len;
				s->file_packets_transferred++;
			}

			netboot_cmd_send(CMD_ACK, &ack, sizeof(ack));

			/* was this the last packet? */
			if(s->file_bytes_transferred == s->file_info->file_size) {
				char16_t *ucs2_path = malloc((strlen(s->file_info->path) + 1) << 1);
				uefi_to_ucs2(s->file_info->path, ucs2_path);

				efi_loaded_image_protocol *loaded_image;
				efi_simple_file_system_protocol *sfs;
				efi_file_protocol *root;
				efi_file_protocol *file;

				efi_status status = st->BootServices->HandleProtocol(handle, &LoadedImageProtocol, (void **) &loaded_image);
				EFIERR(status);

				status = st->BootServices->HandleProtocol(loaded_image->DeviceHandle, &SimpleFileSystemProtocol, (void **) &sfs);
				EFIERR(status);

				status = sfs->OpenVolume(sfs, &root);
				EFIERR(status);

				status = root->Open(root, &file, ucs2_path, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
				EFIERR(status);

				status = file->Write(file, &s->file_info->file_size, s->file_buffer);
				EFIERR(status);

				file->Close(file);

				efi_printf("Transferred file %s\n", s->file_info->path);

				memset(&s->file_info, 0, offsetof(vy_netboot_t, file_packets_transferred) + sizeof(s->file_packets_transferred) - offsetof(vy_netboot_t, file_info));

				s->state = STATE_CMD_WAIT;
			}
		} else {
			efi_printf("unexpected command in file transfer\n");
		}
	}

	if(msg->cmd == CMD_FWSETUP) {
		net_poll_continue = false;

		efi_guid global_variable_guid = {0x8be4df61, 0x93ca, 0x11d2, {0xaa, 0x0d, 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c}};
		size_t var_size;
		uintptr_t var;

		efi_status status = st->RuntimeServices->GetVariable((char16_t *) L"OsIndicationsSupported", &global_variable_guid, NULL, &var_size, &var);
		if(EFI_ERROR(status)) {
			return EFI_SUCCESS;
		}

		var = 1UL;

		status = st->RuntimeServices->SetVariable((char16_t *) L"OsIndications", &global_variable_guid, 7, var_size, &var);
		st->RuntimeServices->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
	} else if(msg->cmd == CMD_STOP_ADVERTISING) {
		net_stop_advertising = true;
	} else if(msg->cmd == CMD_SHUTDOWN) {
		net_poll_continue = false;
		st->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
	} else if(msg->cmd == CMD_REBOOT) {
		net_poll_continue = false;
		st->RuntimeServices->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
	} else if(msg->cmd == CMD_FILE_SEND_INIT) {
		st->BootServices->AllocatePool(EfiLoaderData, msg->message_length - sizeof(vy_netboot_message_t), (void **) &s->file_info);
		memcpy(s->file_info, msg->data, msg->message_length - sizeof(vy_netboot_message_t));

		s->state = STATE_FILE_TRANSFER;

		st->BootServices->AllocatePool(EfiLoaderData, s->file_info->file_size, (void **) &s->file_buffer);

		vy_netboot_ack_t ack = { .type = CMD_FILE_SEND_INIT };
		netboot_cmd_send(CMD_ACK, &ack, sizeof(ack));
	} else if(msg->cmd == CMD_BOOT) {
		net_poll_continue = false;
	}

	return EFI_SUCCESS;
}
