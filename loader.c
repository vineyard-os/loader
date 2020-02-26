#include <efi.h>
#include <efi/protocol/file.h>
#include <cpu.h>
#include <elf.h>
#include <elf64.h>
#include <fs.h>
#include <virt.h>
#include <uefi.h>
#include <stdbool.h>
#include <string.h>

efi_handle handle;
efi_system_table *st;

efi_status efi_main(efi_handle image_handle, efi_system_table *systab) {
	efi_status status;

	st = systab;
	handle = image_handle;

	/* OVMF (and possibly other UEFI implementations) set the WP bit in cr0, which prevents us from modifying page tables */
	virt_wp_disable();

	/* enable NX */
	efi_msr_write(0xC0000080, efi_msr_read(0xC0000080) | 0x800);

	status = st->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
	EFIERR(status);

	status = st->ConOut->ClearScreen(st->ConOut);
	EFIERR(status);

	efi_file_protocol *kernel;
	status = fs_load_kernel(&kernel);
	EFIERR(status);

	/* map a complete copy of the binary into memory (used for parsing by the kernel itself) */
	void *copy = 0;
	efi_file_info *info = NULL;
	efi_guid guid = EFI_FILE_INFO_GUID;
	uintptr_t info_len = 0x1;
	status = kernel->GetInfo(kernel, &guid, &info_len, info);

	if(status != EFI_SUCCESS && status != EFI_BUFFER_TOO_SMALL) {
		efi_panic(L"couldn't get info", status);
	}

	status = st->BootServices->AllocatePool(EfiLoaderData, info_len, (void **) &info);
	EFIERR(status);

	status = kernel->GetInfo(kernel, &guid, &info_len, info);

	status = st->BootServices->AllocatePool(EfiLoaderData, info->FileSize, &copy);

	if(EFI_ERROR(status)) {
		efi_panic(L"unable to allocate space for kernel", status);
	}

	status = kernel->Read(kernel, &info->FileSize, copy);
	EFIERR(status);

	status = kernel->SetPosition(kernel, 0);
	EFIERR(status);

	elf64_ehdr_t header;
	status = fs_read_header(kernel, &header);
	EFIERR(status);

	status = elf_header_check(&header);
	EFIERR(status);

	elf64_phdr_t *phdrs;
	fs_read_phdrs(kernel, header, &phdrs);

	for(elf64_phdr_t *phdr = phdrs; (char *) phdr < (char *) phdrs + header.e_phnum * header.e_phentsize; phdr = (elf64_phdr_t *) ((char *) phdr + header.e_phentsize)) {
		if(phdr->p_type == PT_LOAD) {
			size_t pages = (phdr->p_memsz + 0xfff) / 0x1000;
			uintptr_t physical = phdr->p_paddr;
			status = st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &physical);

			if(EFI_ERROR(status)) {
				efi_print(efi_get_error(status));
				efi_panic(L" error mapping kernel", status);
			}

			kernel->SetPosition(kernel, phdr->p_offset);
			uint64_t size = phdr->p_filesz;
			kernel->Read(kernel, &size, (void *) physical);
			uintptr_t flags = efi_elf_phdr_flags(phdr->p_flags);

			virt_map(physical, phdr->p_vaddr, pages, flags);

			if(phdr->p_memsz - phdr->p_filesz > 0) {
				memset((uint8_t *) physical + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
			}
		}
	}

	status = st->BootServices->FreePool(phdrs);

	if(EFI_ERROR(status)) {
		efi_panic(L"unable to free program headers", status);
	}

	status = kernel->Close(kernel);

	if(EFI_ERROR(status)) {
		efi_panic(L"error closing kernel file", status);
	}

	uintptr_t stack;
	status = st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 33, &stack);

	if(EFI_ERROR(status)) {
		efi_panic(L"unable to allocate page", status);
	}

	virt_map(stack, STACK, 33, PAGE_PRESENT | PAGE_WRITE | PAGE_NX);

	/* we need the assembly stub because we are changing out the stack, so a simple __attribute__((sysv_abi)) won't do the trick here */
	elf_jump_to_kernel(handle, st, (uintptr_t) copy, info->FileSize, header.e_entry);

	for(;;);

	return EFI_SUCCESS;
}
