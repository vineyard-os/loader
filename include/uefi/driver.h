#pragma once

typedef struct {
	efi_device_path_protocol header;
	char16_t path_name[0];
} uefi_filepath_device_path;

efi_status uefi_driver_start(efi_handle image, char16_t *path);
