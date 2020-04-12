#include <uefi.h>
#include <string.h>

char16_t *uefi_handle_get_string(efi_handle h) {
	efi_device_path_protocol *path;

	efi_status status = st->BootServices->HandleProtocol(h, &DevicePathProtocol, (void *) &path);

	if(EFI_ERROR(status)) {
		char16_t *err;
		status = st->BootServices->AllocatePool(EfiLoaderData, sizeof(L"<null_path>"), (void **) &err);
		EFIERR(status);

		memcpy(err, L"<null_path", sizeof(L"<null_path>"));
		return err;
	}

	char16_t *str = uefi_devpath_get_string(path);

	if(!str) {
		char16_t *err;
		status = st->BootServices->AllocatePool(EfiLoaderData, sizeof(L"<null_string>"), (void **) &err);
		EFIERR(status);

		memcpy(err, L"<null_string>", sizeof(L"<null_string>"));
		return err;
	}

	return str;
}

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

int uefi_guid_compare(efi_guid *a, efi_guid *b) {
	if(a->data1 == b->data1 && a->data2 == b->data2 && a->data3 == b->data3) {
		if(*(uint64_t *) &a->data4 == *(uint64_t *) &b->data4) {
			return 0;
		}
	}

	return 1;
}

char uefi_read_keystroke(void) {
	uintptr_t index;
	efi_input_key key = { };
	efi_status s;

	while(!key.UnicodeChar) {
		s = st->BootServices->WaitForEvent(1, &st->ConIn->WaitForKey, &index);
		EFIERR(s);

		s = st->ConIn->ReadKeyStroke(st->ConIn, &key);
		EFIERR(s);
	}

	return (char) (key.UnicodeChar & 0xFFU);
}
