#include <boot/framebuffer.h>
#include <boot/image.h>
#include <boot/memory_map.h>
#include <boot/rsdp.h>
#include <loader.h>
#include <fs.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>

void loader_kernel(void) {
	uint64_t *pml4 = loader_paging_setup();

	efi_file_protocol *kernel = NULL;
	efi_status status = fs_open(&kernel, (char16_t *) L"kernel");
	EFIERR(status);

	efi_file_info *file_info = fs_get_file_info(kernel);

	/* map a complete copy of the binary into memory (used for parsing by the kernel itself) */
	void *copy = malloc(file_info->FileSize);
	fs_read(kernel, file_info->FileSize, 0, copy);

	elf64_ehdr_t header;
	status = loader_parse_header(kernel, &header);
	EFIERR(status);

	status = elf_header_check(&header);
	EFIERR(status);

	loader_kernel_phdrs(kernel, &header, pml4);

	status = kernel->Close(kernel);

	if(EFI_ERROR(status)) {
		efi_panic(L"error closing kernel file", status);
	}

	uintptr_t *stack = uefi_alloc_pages(33);

	if(EFI_ERROR(status)) {
		efi_panic(L"unable to allocate page", status);
	}

	info_t *info = uefi_alloc_pages((sizeof(info_t) >> 12) + 1);
	loader_paging_map(pml4, (uintptr_t) info, (uintptr_t) info, PAGE_WRITE | PAGE_PRESENT);

	info->st = st;
	info->handle = handle;
	info->copy = (uintptr_t) copy;
	info->copy_size = file_info->FileSize;

	for(size_t i = 0; i < 33; i++) {
		loader_paging_map(pml4, (uintptr_t) stack + (i << 12), STACK + (i << 12), PAGE_PRESENT | PAGE_WRITE | PAGE_NX);
	}

	uint64_t *args = (uint64_t *) ((uintptr_t) stack + STACK_SIZE - (8 * 4));
	args[0] = (uintptr_t) info;

	framebuffer_get(info, pml4);
	efi_get_image(info);

	/* map the loader in the new tables */
	size_t pages = info->image->ImageSize >> 12;

	if(info->image->ImageSize % 0x1000) {
		pages++;
	}

	for(size_t i = 0; i < pages; i++) {
		loader_paging_map(pml4, (uintptr_t) info->image->ImageBase + (i << 12), (uintptr_t) info->image->ImageBase + (i << 12), PAGE_PRESENT | PAGE_WRITE);
	}

	efi_get_rsdp(info);
	efi_get_memory_map(info, pml4);

	// we need the assembly stub because we are changing out stack and page tables
	elf_jump_to_kernel(info, header.e_entry, pml4);
}

void loader_kernel_phdrs(efi_file_protocol *kernel, elf64_ehdr_t *header, uint64_t *pml4) {
	elf64_phdr_t *phdrs;
	loader_parse_phdrs(kernel, header, &phdrs);

	for(elf64_phdr_t *phdr = phdrs; (char *) phdr < (char *) phdrs + header->e_phnum * header->e_phentsize; phdr = (elf64_phdr_t *) ((char *) phdr + header->e_phentsize)) {
		if(phdr->p_type == PT_LOAD) {
			size_t pages = (phdr->p_memsz + 0xfff) / 0x1000;
			uintptr_t physical = phdr->p_paddr;
			efi_status status = st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &physical);

			if(EFI_ERROR(status)) {
				efi_print(uefi_strerror(status));
				efi_panic(L" error mapping kernel", status);
			}

			kernel->SetPosition(kernel, phdr->p_offset);
			uint64_t size = phdr->p_filesz;
			kernel->Read(kernel, &size, (void *) physical);
			uintptr_t flags = efi_elf_phdr_flags(phdr->p_flags);

			for(size_t i = 0; i < pages; i++) {
				loader_paging_map(pml4, physical + (i << 12), phdr->p_vaddr + (i << 12), flags);
			}


			if(phdr->p_memsz - phdr->p_filesz > 0) {
				memset((uint8_t *) physical + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
			}
		}
	}

	free(phdrs);
}
