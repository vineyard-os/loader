#include <boot/memory_map.h>
#include <uefi.h>
#include <loader.h>

efi_status efi_get_memory_map(info_t *info, uint64_t *pml4) {
	size_t size = 0;
	size_t descriptor_size;
	uint32_t descriptor_version;
	efi_status status;

	status = st->BootServices->GetMemoryMap(&size, info->efi_memory_map, &info->efi_memory_map_key, &descriptor_size, &descriptor_version);

	if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
		return status;
	}

	status = st->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, ((size + 0xFFF) / 0x1000) + 1, (efi_physical_addr *) &info->efi_memory_map);

	if(EFI_ERROR(status)) {
		return status;
	}

	loader_paging_map_addr(pml4, (uintptr_t) info->efi_memory_map, (((size + 0xFFF) / 0x1000) + 1) << 12, PAGE_WRITE | PAGE_PRESENT);

	status = st->BootServices->GetMemoryMap(&size, info->efi_memory_map, &info->efi_memory_map_key, &descriptor_size, &descriptor_version);

	if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
		return status;
	}

	info->efi_memory_map_entries = size / descriptor_size;
	info->efi_memory_map_descriptor_size = descriptor_size;

	status = st->BootServices->ExitBootServices(handle, info->efi_memory_map_key);
	EFIERR(status);

	return EFI_SUCCESS;
}
