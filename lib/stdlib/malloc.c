#include <uefi.h>
#include <stdlib.h>

void *malloc(size_t size) {
	void *pointer = NULL;

	efi_status s = st->BootServices->AllocatePool(EfiLoaderData, size, &pointer);
	EFIERR(s);

	return pointer;
}
