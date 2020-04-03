#include <uefi.h>
#include <string.h>

efi_status uefi_driver_start(efi_handle image, char16_t *path) {
	efi_device_path_protocol *protocol = uefi_devpath_from_path(image, path);

	efi_handle driver_handle;
	efi_loaded_image_protocol *driver_image;

	efi_status status = st->BootServices->LoadImage(false, image, protocol, NULL, 0, &driver_handle);
	EFIERR(status);

	status = st->BootServices->HandleProtocol(driver_handle, &LoadedImageProtocol, (void **) &driver_image);
	EFIERR(status);

	driver_image->LoadOptions = (void *) L"";
	driver_image->LoadOptionsSize = (uint32_t) efi_strlen(driver_image->LoadOptions);

	status = st->BootServices->StartImage(driver_handle, NULL, NULL);
	EFIERR(status);

	return EFI_SUCCESS;
}
