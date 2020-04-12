#pragma once

char16_t *uefi_handle_get_string(efi_handle h);
void uefi_key_wait(void);
efi_status uefi_get_boot_device(efi_handle image);
void *uefi_alloc_pages(size_t pages);
int uefi_guid_compare(efi_guid *a, efi_guid *b);
char uefi_read_keystroke(void);

const char *uefi_strerror(efi_status status);

void uefi_to_ucs2(char *utf8, char16_t *ucs2);
