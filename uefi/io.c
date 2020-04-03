#include <stddef.h>
#include <string.h>
#include <uefi.h>

#define EFI_HANDLE_TYPE_UNKNOWN 0x000
#define EFI_HANDLE_TYPE_IMAGE_HANDLE 0x001
#define EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE 0x002
#define EFI_HANDLE_TYPE_DEVICE_DRIVER 0x004
#define EFI_HANDLE_TYPE_BUS_DRIVER 0x008
#define EFI_HANDLE_TYPE_DRIVER_CONFIGURATION_HANDLE 0x010
#define EFI_HANDLE_TYPE_DRIVER_DIAGNOSTICS_HANDLE 0x020
#define EFI_HANDLE_TYPE_COMPONENT_NAME_HANDLE 0x040
#define EFI_HANDLE_TYPE_DEVICE_HANDLE 0x080
#define EFI_HANDLE_TYPE_PARENT_HANDLE 0x100
#define EFI_HANDLE_TYPE_CONTROLLER_HANDLE 0x200
#define EFI_HANDLE_TYPE_CHILD_HANDLE 0x400

static efi_status uefi_io_parse_handles(efi_handle controller_handle, size_t *handle_count, efi_handle **handle_buffer, uint32_t **handle_type) {
	*handle_count = 0;
	*handle_buffer = NULL;
	*handle_type = NULL;

	efi_status status = st->BootServices->LocateHandleBuffer(AllHandles, NULL, NULL, handle_count, handle_buffer);
	EFIERR(status);

	status = st->BootServices->AllocatePool(EfiLoaderData, *handle_count * sizeof(uint32_t), (void **) handle_type);
	EFIERR(status);

	efi_guid **protocol_guid_array;
	size_t array_count;

	for(size_t i = 0; i < *handle_count; i++) {
		(*handle_type)[i] = EFI_HANDLE_TYPE_UNKNOWN;

		status = st->BootServices->ProtocolsPerHandle((*handle_buffer)[i], &protocol_guid_array, &array_count);
		EFIERR(status);

		for(size_t j = 0; j < array_count; j++) {
			if(!uefi_guid_compare(protocol_guid_array[j], &LoadedImageProtocol)) {
				(*handle_type)[i] |= EFI_HANDLE_TYPE_IMAGE_HANDLE;
			}

			if(!uefi_guid_compare(protocol_guid_array[j], &DriverBindingProtocol)) {
				(*handle_type)[i] |= EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE;
			}

			if(!uefi_guid_compare(protocol_guid_array[j], &DevicePathProtocol)) {
				(*handle_type)[i] |= EFI_HANDLE_TYPE_DEVICE_HANDLE;
			}

			efi_open_protocol_information_entry *open_info;
			size_t open_info_count;

			status = st->BootServices->OpenProtocolInformation((*handle_buffer)[i], protocol_guid_array[j], &open_info, &open_info_count);
			EFIERR(status);

			for(size_t k = 0; k < open_info_count; k++) {
				if(open_info[k].controller_handle == controller_handle) {
					if((open_info[k].attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
						(*handle_type)[i] |= EFI_HANDLE_TYPE_PARENT_HANDLE;
					}
				}
			}
		}
	}

	return EFI_SUCCESS;
}

void uefi_io_reconnect(void) {
	size_t all_handle_count;
	efi_handle *all_handle_buffer;

	efi_status status = st->BootServices->LocateHandleBuffer(AllHandles, NULL, NULL, &all_handle_count, &all_handle_buffer);
	EFIERR(status);

	for(size_t i = 0; i < all_handle_count; i++) {
		size_t handle_count;
		efi_handle *handle_buffer;
		uint32_t *handle_type;

		status = uefi_io_parse_handles(all_handle_buffer[i], &handle_count, &handle_buffer, &handle_type);
		EFIERR(status);

		bool device = true;

		if(handle_type[i] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE || handle_type[i] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
			device = false;
		} else {
			bool parent = false;

			for(size_t handle_i = 0; handle_i < handle_count; handle_i++) {
				if(handle_type[handle_i] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
					parent = true;
				}
			}

			if(!parent) {
				if(handle_type[i] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
					st->BootServices->ConnectController(all_handle_buffer[i], NULL, NULL, true);
				}
			}
		}
	}
}
