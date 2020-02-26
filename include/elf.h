#pragma once

#include <elf64.h>
#include <stdint.h>
#include <uefi.h>

uintptr_t efi_elf_phdr_flags(uintptr_t flag);
efi_status elf_header_check(elf64_ehdr_t *header);
void elf_jump_to_kernel(efi_handle handle, efi_system_table *st, uintptr_t kernel, size_t kernel_size, elf64_addr_t entry);
