#pragma once

#include <efi.h>
#include <elf64.h>

efi_status fs_load_kernel(efi_file_protocol **kernel);
efi_status fs_read_header(efi_file_protocol *kernel, elf64_ehdr_t *header);
efi_status fs_read_phdrs(efi_file_protocol *kernel, elf64_ehdr_t header, elf64_phdr_t **phdrs);
