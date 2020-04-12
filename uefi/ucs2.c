#include <uefi.h>

void uefi_to_ucs2(char *utf8, char16_t *ucs2) {
	size_t utf8i = 0;
	size_t ucs2i = 0;

	while(utf8[utf8i]) {
		char c0 = utf8[utf8i];
		uint16_t value;

		if(!(c0 & 0x80)) {
			utf8i += 1;

			value = (uint16_t) c0;
		} else if((c0 & 0xE0) == 0xC0) {
			utf8i += 2;

			value = (uint16_t) ((c0 & 0x1F) << 6);
			value |= (utf8[utf8i + 1] & 0x3F);
		} else if((c0 & 0xF0) == 0xE0) {
			utf8i += 3;

			value = (uint16_t) ((c0 & 0xF) << 12);
			value |= (utf8[utf8i + 1] & 0x3F) << 6;
			value |= (utf8[utf8i + 2] & 0x3F);
		} else {
			utf8i += 4;
			value = 0xFFFC;
		}

		ucs2[ucs2i++] = value;
	}

	ucs2[ucs2i] = L'\0';
}
