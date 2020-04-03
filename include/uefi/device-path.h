#pragma once

char16_t *uefi_devpath_get_string(efi_device_path_protocol *path);
void uefi_devpath_print(efi_device_path_protocol *path);

bool uefi_devpath_valid(const efi_device_path_protocol *device);
size_t uefi_devpath_size(efi_device_path_protocol *device);
bool uefi_devpath_is_end(const efi_device_path_protocol *node);
void uefi_devpath_set_end(void *node);
void uefi_devpath_set_node_length(efi_device_path_protocol *node, size_t length);
size_t uefi_devpath_get_node_length(efi_device_path_protocol *node);

efi_device_path_protocol *uefi_devpath_get_next(efi_device_path_protocol *node);
efi_device_path_protocol *uefi_devpath_append(efi_device_path_protocol *first, efi_device_path_protocol *second);

efi_device_path_protocol *uefi_devpath_from_device_handle(efi_handle h);
efi_device_path_protocol *uefi_devpath_from_file_handle(efi_handle handle);
efi_device_path_protocol *uefi_devpath_from_path(efi_handle device, const char16_t *filename);
