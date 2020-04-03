#include <loader.h>

efi_status loader_parse_header(efi_file_protocol *kernel, elf64_ehdr_t *header) {
	efi_status status;

	uint64_t size = sizeof(*header);
	status = kernel->Read(kernel, &size, header);
	EFIERR(status);

	return EFI_SUCCESS;
}

efi_status loader_parse_phdrs(efi_file_protocol *kernel, elf64_ehdr_t *header, elf64_phdr_t **phdrs) {
	efi_status status = kernel->SetPosition(kernel, header->e_phoff);
	EFIERR(status);
	uint64_t size = (size_t) header->e_phnum * header->e_phentsize;
	status = st->BootServices->AllocatePool(EfiLoaderData, size, (void **) phdrs);
	EFIERR(status);
	status = kernel->Read(kernel, &size, *phdrs);
	EFIERR(status);

	return EFI_SUCCESS;
}
