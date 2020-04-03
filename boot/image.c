#include <boot/image.h>

efi_status efi_get_image(info_t *info) {
	efi_status status;
	efi_guid guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

	status = info->st->BootServices->HandleProtocol(info->handle, &guid, (void **) &info->image);
	EFIERR(status);

	return EFI_SUCCESS;
}
