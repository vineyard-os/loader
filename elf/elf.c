#include <elf.h>
#include <loader.h>
#include <uefi.h>

uintptr_t efi_elf_phdr_flags(uintptr_t f) {
	uintptr_t flags = PAGE_PRESENT;

	if(!(f | PT_X)) {
		flags |= PAGE_NX;
	}

	if(f | PT_W) {
		flags |= PAGE_WRITE;
	}

	return flags;
}

efi_status elf_header_check(elf64_ehdr_t *header) {
	if(header->e_ident[EI_MAG0] != EI_MAG0_VALUE && header->e_ident[EI_MAG1] != EI_MAG1_VALUE && header->e_ident[EI_MAG2] != EI_MAG2_VALUE && header->e_ident[EI_MAG3] != EI_MAG3_VALUE) {
		efi_panic(L"invalid ELF magic", EFI_INVALID_PARAMETER);
	}

	if(header->e_ident[EI_CLASS] != ELF_CLASS_64) {
		efi_panic(L"invalid ELF class", EFI_INVALID_PARAMETER);
	}

	if(header->e_ident[EI_DATA] != ELF_DATA_2LSB) {
		efi_panic(L"invalid ELF endianness", EFI_INVALID_PARAMETER);
	}

	if(header->e_ident[EI_VERSION] != 1) {
		efi_panic(L"invalid ELF version", EFI_INVALID_PARAMETER);
	}

	if(header->e_ident[EI_OSABI] != ELF_OSABI_SYSTEMV) {
		efi_panic(L"invalid ELF OS ABI", EFI_INVALID_PARAMETER);
	}

	if(header->e_type != ET_EXEC) {
		efi_panic(L"invalid ELF type", EFI_INVALID_PARAMETER);
	}

	if(header->e_machine != EM_X86_64) {
		efi_panic(L"invalid ELF machine type", EFI_INVALID_PARAMETER);
	}

	return EFI_SUCCESS;
}
