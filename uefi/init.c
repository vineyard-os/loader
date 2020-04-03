#include <cpu.h>
#include <loader.h>
#include <uefi.h>

void uefi_init(void) {
	/* OVMF (and possibly other UEFI implementations) set the WP bit in cr0, which prevents us from modifying page tables */
	loader_paging_wp_disable();

	/* enable NX */
	efi_msr_write(0xC0000080, efi_msr_read(0xC0000080) | 0x800);

	efi_status status = st->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
	EFIERR(status);

	status = st->ConOut->ClearScreen(st->ConOut);
	EFIERR(status);
}
