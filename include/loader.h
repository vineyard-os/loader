#pragma once

#include <elf64.h>
#include <uefi.h>

#define PAGE_PRESENT 0x01
#define PAGE_WRITE 0x02
#define PAGE_NX 0x8000000000000000

void loader_kernel(void);
void loader_kernel_phdrs(efi_file_protocol *kernel, elf64_ehdr_t *header, uint64_t *pml4);

efi_status loader_parse_header(efi_file_protocol *kernel, elf64_ehdr_t *header);
efi_status loader_parse_phdrs(efi_file_protocol *kernel, elf64_ehdr_t *header, elf64_phdr_t **phdrs);

uint64_t *loader_paging_setup(void);
void loader_paging_map(uint64_t *pml4, uintptr_t physical, uintptr_t virtual, uintptr_t flags);
void loader_paging_map_addr(uint64_t *pml4, uintptr_t start, size_t length, uintptr_t flags);
void loader_paging_wp_disable(void);
