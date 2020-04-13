#include <uefi.h>
#include <string.h>

static const efi_device_path_protocol empty_devpath = {
	DEVICE_PATH_END,
	DEVICE_PATH_ENTIRE_END,
	{ sizeof(efi_device_path_protocol), 0 },
};

char16_t *uefi_devpath_get_string(efi_device_path_protocol *path) {
	efi_device_path_to_text_protocol* ptt;
	efi_status status = st->BootServices->LocateProtocol(&DevicePathToTextProtocol, NULL, (void**)&ptt);

	if(EFI_ERROR(status)) {
		return NULL;
	}

	return ptt->ConvertDevicePathToText(path, false, false);
}

void uefi_devpath_print(efi_device_path_protocol *path) {
	st->ConOut->OutputString(st->ConOut, uefi_devpath_get_string(path));
}

void uefi_devpath_set_node_length(efi_device_path_protocol *node, size_t length) {
	if(!node || length < sizeof(efi_device_path_protocol)) {
		efi_panic("%s: error", __func__);
	}

	*(uint16_t *) &node->Length[0] = (uint16_t) length;
}

size_t uefi_devpath_get_node_length(efi_device_path_protocol *node) {
	return *(uint16_t *) &node->Length[0];
}

efi_device_path_protocol *uefi_devpath_get_next(efi_device_path_protocol *node) {
	return (efi_device_path_protocol *) ((uintptr_t) node + uefi_devpath_get_node_length(node));
}

void uefi_devpath_set_end(void *node) {
	if(!node) {
		efi_panic("%s: error", __func__);
	}

	memcpy(node, &empty_devpath, sizeof(empty_devpath));
}

efi_device_path_protocol *uefi_devpath_from_device_handle(efi_handle h) {
	efi_device_path_protocol *protocol;

	efi_status status = st->BootServices->HandleProtocol(h, &DevicePathProtocol, (void *) &protocol);
	EFIERR(status);

	return protocol;
}

efi_device_path_protocol *uefi_devpath_from_file_handle(efi_handle h) {
	efi_device_path_protocol *devpath;
	efi_loaded_image_protocol *protocol;

	efi_status status = st->BootServices->OpenProtocol(h, &LoadedImageProtocol, (void **) &protocol, h, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

	if(EFI_ERROR(status)) {
		efi_printf("uefi_devpath_from_file_handle: open LoadedImageProtocol status %zu\n", status);
		for(;;);
	}

	status = st->BootServices->OpenProtocol(protocol->DeviceHandle, &DevicePathProtocol, (void **) &devpath, h, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

	if(EFI_ERROR(status)) {
		efi_printf("uefi_devpath_from_file_handle: open DevicePathProtocol status %zu\n", status);
		for(;;);
	}

	return devpath;
}

efi_device_path_protocol *uefi_devpath_from_path(efi_handle device, const char16_t *filename) {
	size_t size = (efi_strlen(filename) + 1) * 2;

	uefi_filepath_device_path *file_path;
	efi_device_path_protocol *devpath = NULL;
	efi_device_path_protocol *file_devpath;

	efi_status status = st->BootServices->AllocatePool(EfiLoaderData, size + offsetof(uefi_filepath_device_path, path_name) + sizeof(efi_device_path_protocol), (void **) &file_devpath);
	EFIERR(status);

	if(file_devpath) {
		file_path = (uefi_filepath_device_path *) file_devpath;
		file_path->header.Type = DEVICE_PATH_MEDIA;
		file_path->header.SubType = DEVICE_PATH_MEDIA;

		memcpy(&file_path->path_name, filename, size);
		file_path->path_name[size] = 0;

		uefi_devpath_set_node_length(&file_path->header, size + offsetof(uefi_filepath_device_path, path_name));
		uefi_devpath_set_end(uefi_devpath_get_next(&file_path->header));

		if(device) {
			devpath = uefi_devpath_from_file_handle(device);
		}

		devpath = uefi_devpath_append(devpath, file_devpath);
	}

	return devpath;
}

bool uefi_devpath_valid(const efi_device_path_protocol *device) {
	if(!device) {
		return false;
	}

	/* TODO: actually check */

	return true;
}

bool uefi_devpath_is_end(const efi_device_path_protocol *node) {
	if(!node) {
		efi_panic("%s: error", __func__);
	}

	if(node->Type == DEVICE_PATH_END && node->SubType == DEVICE_PATH_ENTIRE_END) {
		return true;
	}

	return false;
}

size_t uefi_devpath_size(efi_device_path_protocol *device) {
	efi_device_path_protocol *node = device;

	while(!uefi_devpath_is_end(node)) {
		node = uefi_devpath_get_next(node);
	}

	return ((uintptr_t) node - (uintptr_t) device) + uefi_devpath_get_node_length(node);
}

efi_device_path_protocol *uefi_devpath_append(efi_device_path_protocol *first, efi_device_path_protocol *second) {
	if(!uefi_devpath_valid(first) || !uefi_devpath_valid(second)) {
		efi_panic("%s: invalid device paths", __func__);
	}

	size_t first_size = uefi_devpath_size(first);
	size_t second_size = uefi_devpath_size(second);

	efi_device_path_protocol *devpath;
	efi_status status = st->BootServices->AllocatePool(EfiLoaderData, first_size + second_size, (void **) &devpath);
	EFIERR(status);

	efi_device_path_protocol *empty_node = (efi_device_path_protocol *) ((uintptr_t) devpath + first_size - sizeof(empty_devpath));

	memcpy(devpath, first, first_size);
	memcpy(empty_node, second, second_size);

	return devpath;
}
