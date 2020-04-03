#include <fs.h>
#include <stdlib.h>
#include <uefi.h>

efi_file_info *fs_get_file_info(efi_file_protocol *file) {
	efi_file_info *file_info = NULL;
	efi_guid guid = EFI_FILE_INFO_GUID;
	uintptr_t info_len = 0x1;

	efi_status status = file->GetInfo(file, &guid, &info_len, file_info);

	if(status != EFI_SUCCESS && status != EFI_BUFFER_TOO_SMALL) {
		efi_panic(L"couldn't get file info", status);
	}


	file_info = malloc(info_len);

	status = file->GetInfo(file, &guid, &info_len, file_info);

	return file_info;
}
