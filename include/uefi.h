#pragma once

#include <efi.h>

#include <uefi/device-path.h>
#include <uefi/driver.h>
#include <uefi/init.h>
#include <uefi/io.h>
#include <uefi/util.h>

#define EFIERR(x) if(EFI_ERROR((x))) { efi_printf((char *) "EFIERR %s @ %s:%d\n", uefi_strerror(x), __FILE__, __LINE__); uefi_key_wait(); st->BootServices->Exit(handle, 1, 0, NULL); }
#define efi_panic(msg, code) { st->ConOut->OutputString(st->ConOut, (char16_t *) (msg)); for(;;); }

extern efi_handle handle;
extern efi_system_table *st;

size_t efi_strlen(const char16_t *str);
efi_status efi_print(const char *str);
efi_status efi_printn(const char *str, size_t n);
__attribute__((format(printf, 1, 2))) int efi_printf(const char *restrict format, ...);

efi_status efi_main(efi_handle image_handle, efi_system_table *systab);
void efi_loader_jump_to_kernel(efi_handle handle, efi_system_table *st, uintptr_t copy, uintptr_t entry);
