#include <cpu.h>
#include <loader.h>
#include <string.h>

struct vm_indices {
	size_t pml4i;
	size_t pml3i;
	size_t pml2i;
	size_t pml1i;
	uint64_t *pml4;
	uint64_t *pml3;
	uint64_t *pml2;
	uint64_t *pml1;
};

static void mm_virtual_indices(struct vm_indices *i, uintptr_t addr) {
	if(addr >= 0xFFFF800000000000) {
		addr -= 0xFFFF800000000000;
		i->pml4i = 256 + ((addr >> 39) & 0x1FF);
	} else {
		i->pml4i = (addr >> 39) & 0x1FF;
	}

	i->pml3i = (addr >> 30) & 0x1FF;
	i->pml2i = (addr >> 21) & 0x1FF;
	i->pml1i = (addr >> 12) & 0x1FF;
}

uint64_t *loader_paging_setup(void) {
	uint64_t *pml4 = uefi_alloc_pages(1);

	pml4[510] = (uintptr_t) pml4 | PAGE_WRITE | PAGE_PRESENT;
	/* TODO: set up PAT table */

	return pml4;
}

void loader_paging_map(uint64_t *pml4, uintptr_t physical, uintptr_t virtual, uintptr_t flags) {
	struct vm_indices i;

	mm_virtual_indices(&i, virtual);

	if(!(pml4[i.pml4i] & PAGE_PRESENT)) {
		i.pml3 = uefi_alloc_pages(1);
		pml4[i.pml4i] = (uintptr_t) i.pml3 | PAGE_PRESENT | PAGE_WRITE;
	} else {
		i.pml3 = (void *) (pml4[i.pml4i] & 0x000FFFFFFFFFF000UL);
	}

	if(!(i.pml3[i.pml3i] & PAGE_PRESENT)) {
		i.pml2 = uefi_alloc_pages(1);
		i.pml3[i.pml3i] = (uintptr_t) i.pml2 | PAGE_PRESENT | PAGE_WRITE;
	} else {
		i.pml2 = (void *) (i.pml3[i.pml3i] & 0x000FFFFFFFFFF000UL);
	}

	if(!(i.pml2[i.pml2i] & PAGE_PRESENT)) {
		i.pml1 = uefi_alloc_pages(1);
		i.pml2[i.pml2i] = (uintptr_t) i.pml1 | PAGE_PRESENT | PAGE_WRITE;
	} else {
		i.pml1 = (void *) (i.pml2[i.pml2i] & 0x000FFFFFFFFFF000UL);
	}

	i.pml1[i.pml1i] = physical | flags;
}

void loader_paging_map_addr(uint64_t *pml4, uintptr_t start, size_t length, uintptr_t flags) {
	size_t start_page = start >> 12;
	size_t end_page = (start + length) >> 12;
	size_t pages = 1 + (end_page - start_page);
	uintptr_t page_addr = start & 0x000FFFFFFFFFF000UL;

	for(size_t i = 0; i < pages; i++) {
		loader_paging_map(pml4, page_addr + (i << 12), page_addr + (i << 12), flags);
	}
}

void loader_paging_wp_disable(void) {
	uint64_t cr0 = efi_cr0_read();
	cr0 &= ~(1UL << 16);
	efi_cr0_write(cr0);
}
