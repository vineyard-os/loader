#pragma once

#include <boot/info.h>
#include <uefi.h>

efi_status efi_get_memory_map(info_t *info, uint64_t *pml4);
