#include <efi.h>
#include <uefi.h>

struct uefi_errors {
	efi_status status;
	const char *text;
} static const uefi_errors[] = {
	{EFI_LOAD_ERROR, "LOAD_ERROR"},
	{EFI_INVALID_PARAMETER, "INVALID_PARAMETER"},
	{EFI_UNSUPPORTED, "UNSUPPORTED"},
	{EFI_BAD_BUFFER_SIZE, "BAD_BUFFER_SIZE"},
	{EFI_BUFFER_TOO_SMALL, "BUFFER_TOO_SMALL"},
	{EFI_NOT_READY, "NOT_READY"},
	{EFI_DEVICE_ERROR, "DEVICE_ERROR"},
	{EFI_WRITE_PROTECTED, "WRITE_PROTECTED"},
	{EFI_OUT_OF_RESOURCES, "OUT_OF_RESOURCES"},
	{EFI_VOLUME_CORRUPTED, "VOLUME_CORRUPTED"},
	{EFI_VOLUME_FULL, "VOLUME_FULL"},
	{EFI_NO_MEDIA, "NO_MEDIA"},
	{EFI_MEDIA_CHANGED, "MEDIA_CHANGED"},
	{EFI_NOT_FOUND, "NOT_FOUND"},
	{EFI_ACCESS_DENIED, "ACCESS_DENIED"},
	{EFI_NO_RESPONSE, "NO_RESPONSE"},
	{EFI_NO_MAPPING, "NO_MAPPING"},
	{EFI_TIMEOUT, "TIMEOUT"},
	{EFI_NOT_STARTED, "NOT_STARTED"},
	{EFI_ALREADY_STARTED, "ALREADY_STARTED"},
	{EFI_ABORTED, "ABORTED"},
	{EFI_ICMP_ERROR, "ICMP_ERROR"},
	{EFI_TFTP_ERROR, "TFTP_ERROR"},
	{EFI_PROTOCOL_ERROR, "PROTOCOL_ERROR"},
	{EFI_INCOMPATIBLE_VERSION, "INCOMPATIBLE_VERSION"},
	{EFI_SECURITY_VIOLATION, "SECURITY_VIOLATION"},
	{EFI_CRC_ERROR, "CRC_ERROR"},
	{EFI_END_OF_MEDIA, "END_OF_MEDIA"},
	{EFI_END_OF_FILE, "END_OF_FILE"},
	{EFI_INVALID_LANGUAGE, "INVALID_LANGUAGE"},
	{EFI_COMPROMISED_DATA, "COMPROMISED_DATA"},
	{EFI_IP_ADDRESS_CONFLICT, "IP_ADDRESS_CONFLICT"},
	{EFI_HTTP_ERROR, "HTTP_ERROR"},
	{0, 0},
};

const char *uefi_strerror(efi_status status) {
	for(size_t i = 0; uefi_errors[i].status; i++) {
		if(uefi_errors[i].status == status) {
			return uefi_errors[i].text;
		}
	}

	return "Unknown EFI error";
}