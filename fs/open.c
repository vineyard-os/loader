#include <fs.h>
#include <uefi.h>

efi_status fs_open(efi_file_protocol **file, char16_t *path) {
	efi_guid loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	efi_guid simple_fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

	efi_loaded_image_protocol *loaded_image;
	efi_status status = st->BootServices->HandleProtocol(handle, &loaded_image_guid, (void **) &loaded_image);
	EFIERR(status);

	efi_simple_file_system_protocol *file_system;
	status = st->BootServices->HandleProtocol(loaded_image->DeviceHandle, &simple_fs_guid, (void **) &file_system);
	EFIERR(status);

	efi_file_protocol *root;
	status = file_system->OpenVolume(file_system, &root);
	EFIERR(status);

	efi_status s = root->Open(root, file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	if(s != EFI_SUCCESS) {
		efi_panic(L"file not found\n", s);
	}

	return EFI_SUCCESS;
}
