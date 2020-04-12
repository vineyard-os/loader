#include <loader.h>
#include <net.h>
#include <uefi.h>

efi_handle handle;
efi_system_table *st;

efi_status efi_main(efi_handle image_handle, efi_system_table *systab) {
	st = systab;
	handle = image_handle;

	uefi_init();

	if((st->FirmwareRevision >> 16) > 1) {
		uefi_driver_start(image_handle, (char16_t *) L"btrfs.efi");
	}

	uefi_io_reconnect();

	efi_printf("vineyard loader\nselect a boot mode:\nn) netboot via netboot-server\nd) local disk\n");

	char c = 0;

	while(c != 'n' && c != 'd') {
		c = uefi_read_keystroke();
	}

	st->ConOut->ClearScreen(st->ConOut);

	if(c == 'n') {
		net_kernel_load();
		st->ConOut->ClearScreen(st->ConOut);
		loader_kernel();
	} else if(c == 'd') {
		loader_kernel();
	}

	return EFI_SUCCESS;
}
