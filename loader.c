#include <loader.h>
#include <uefi.h>

efi_handle handle;
efi_system_table *st;

efi_status efi_main(efi_handle image_handle, efi_system_table *systab) {
	st = systab;
	handle = image_handle;

	uefi_init();



	loader_kernel();

	return EFI_SUCCESS;
}
