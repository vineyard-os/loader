#include <boot/framebuffer.h>
#include <loader.h>
#include <uefi.h>
#include <string.h>

#define EFI_OVMF_SIGNATURE "E\0D\0K\0 \0I\0I\0\0"

static efi_status efi_gop_get(efi_graphics_output_protocol **out, info_t *info) {
	efi_guid gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

	uint64_t num;
	efi_handle *handles;
	efi_status status;

	status = info->st->BootServices->LocateHandleBuffer(ByProtocol, &gop_guid, NULL, &num, &handles);
	EFIERR(status);

	for(uint64_t i = 0; i < num; ++i) {
		efi_graphics_output_protocol *gop;
		status = info->st->BootServices->OpenProtocol(handles[i], &gop_guid, (void **) &gop, info->handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
		EFIERR(status);

		if(gop) {
			*out = gop;
			return EFI_SUCCESS;
		}
	}

	out = NULL;

	return EFI_UNSUPPORTED;
}

static bool framebuffer_setup(efi_graphics_output_protocol *gop, info_t *info) {
	size_t max_w = 0;
	size_t max_h = 0;
	uint32_t selection = 0;
	efi_status status;

	for(uint32_t i = 0; i < gop->Mode->MaxMode; i++) {
		efi_graphics_output_mode_information *mode_info;
		size_t info_size = sizeof(mode_info);
		info->st->BootServices->AllocatePool(EfiLoaderData, info_size, (void **) &mode_info);

		status = gop->QueryMode(gop, i, &info_size, &mode_info);
		EFIERR(status);

		size_t width = mode_info->HorizontalResolution;
		size_t h = mode_info->VerticalResolution;

		if(!memcmp(EFI_OVMF_SIGNATURE, info->st->FirmwareVendor, 13) && width == 800 && h == 600) {
			gop->SetMode(gop, i);

			return true;
		} else if(width > max_w || h > max_h) {
			max_w = width;
			max_h = h;
			selection = i;
		}
	}

	if(memcmp(EFI_OVMF_SIGNATURE, info->st->FirmwareVendor, 13) && selection != 0) {
		gop->SetMode(gop, selection);

		return true;
	}

	return false;
}

void framebuffer_get(info_t *info, uint64_t *pml4) {
	efi_graphics_output_protocol *gop;

	efi_gop_get(&gop, info);
	framebuffer_setup(gop, info);

	info->gop_mode = gop->Mode;
	loader_paging_map_addr(pml4, (uintptr_t) info->gop_mode, sizeof(info->gop_mode), PAGE_PRESENT | PAGE_WRITE);
	loader_paging_map_addr(pml4, (uintptr_t) info->gop_mode->Info, sizeof(info->gop_mode->Info), PAGE_PRESENT | PAGE_WRITE);

	size_t framebuffer_pages = (gop->Mode->FrameBufferSize >> 12);

	if(gop->Mode->FrameBufferSize % 0x1000) {
		framebuffer_pages++;
	}

	for(size_t i = 0; i < framebuffer_pages; i++) {
		loader_paging_map(pml4, gop->Mode->FrameBufferBase + (i << 12), gop->Mode->FrameBufferBase + (i << 12), PAGE_PRESENT | PAGE_WRITE);
	}
}
