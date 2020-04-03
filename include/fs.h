#pragma once

#include <efi.h>
#include <elf64.h>

efi_status fs_open(efi_file_protocol **kernel, char16_t *path);
void fs_read(efi_file_protocol *file, size_t len, size_t offset, void *buf);

efi_file_info *fs_get_file_info(efi_file_protocol *file);
