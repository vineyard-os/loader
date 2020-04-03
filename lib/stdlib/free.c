#include <uefi.h>
#include <stdlib.h>

void free(void *pointer) {
	efi_status s = st->BootServices->FreePool(pointer);
	EFIERR(s);
}
