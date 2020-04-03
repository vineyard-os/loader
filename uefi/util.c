#include <uefi.h>
#include <string.h>

void uefi_key_wait(void) {
	efi_simple_text_input_protocol* sii = st->ConIn;
	efi_input_key key;
	while(sii->ReadKeyStroke(sii, &key) != EFI_SUCCESS);
}

void *uefi_alloc_pages(size_t pages) {
	uintptr_t addr;

	efi_status status = st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &addr);
	EFIERR(status);

	memset((void *) addr, 0, pages << 12UL);

	return (void *) addr;
}
