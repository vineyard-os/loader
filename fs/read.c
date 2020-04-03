#include <fs.h>
#include <uefi.h>

void fs_read(efi_file_protocol *file, size_t len, size_t offset, void *buf) {
	efi_status status = file->SetPosition(file, offset);
	EFIERR(status);

	status = file->Read(file, &len, buf);
	EFIERR(status);

	status = file->SetPosition(file, 0);
	EFIERR(status);
}
