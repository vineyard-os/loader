#pragma once

#include <elf64.h>
#include <stdint.h>
#include <uefi.h>

uintptr_t efi_elf_phdr_flags(uintptr_t flag);
efi_status elf_header_check(elf64_ehdr_t *header);
__attribute__((sysv_abi)) void elf_jump_to_kernel(void *data, elf64_addr_t entry, uint64_t *pml4);
