EFI_CFLAGS		?= -g
EFI_CFLAGS		:= $(EFI_CFLAGS) -pipe -Wall -Wbad-function-cast -Wconversion -Werror -Wextra -Wformat=2 -Wformat-security -Winit-self
EFI_CFLAGS		:= $(EFI_CFLAGS) -Wparentheses -Winline -Wmissing-braces -Wmissing-declarations -Wmissing-field-initializers
EFI_CFLAGS		:= $(EFI_CFLAGS) -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes
EFI_CFLAGS		:= $(EFI_CFLAGS) -Wswitch-default -Wswitch-enum -Wuninitialized -Wunreachable-code -Wunused -Wwrite-strings
EFI_CFLAGS		:= $(EFI_CFLAGS) -mno-red-zone -ffreestanding -fno-stack-protector -MMD -MP -std=gnu11 -mno-mmx -mno-sse -mno-sse2 -DVINEYARD
EFI_CFLAGS		:= $(EFI_CFLAGS) -Iinclude -Wno-format -Wno-format-security -D STACK=0xFFFFFE0000000000 -D STACK_SIZE=0x21000 -D VINEYARD_LOADER
EFI_ASFLAGS		:= -fwin64 -D STACK=0xFFFFFE0000000000 -D STACK_SIZE=0x21000

EFI_CLANG		:= clang
EFI_GCC			:= x86_64-w64-mingw32-gcc
EFI_LLD-LINK	:= lld-link
EFI_AS			:= yasm

ifndef USE_GCC
	EFI_CC		:= $(EFI_CLANG) -target x86_64-pc-win32-coff
	EFI_LD		:= $(EFI_LLD-LINK)
	EFI_CFLAGS	:= $(EFI_CFLAGS) -Wmissing-variable-declarations -Wused-but-marked-unused -flto
	EFI_LDFLAGS	 = -subsystem:efi_application -nodefaultlib -dll -WX -entry:efi_main -out:$(LOADER)
else
	EFI_CC		:= $(EFI_GCC)
	EFI_LD		:= $(EFI_CC)
	EFI_CFLAGS	:= $(EFI_CFLAGS) -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2
	EFI_LDFLAGS	 = -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $(LOADER)
endif

LOADER_SRC_C	:= $(shell find -name '*.c' -type f)
LOADER_SRC_ASM	:= $(shell find -name '*.s' -type f)
LOADER_SRC		:= $(LOADER_SRC_C) $(LOADER_SRC_ASM)
LOADER_OBJ_C	:= $(addprefix bin/obj/,$(addsuffix .o,$(LOADER_SRC_C)))
LOADER_OBJ		:= $(addprefix bin/obj/,$(addsuffix .o,$(LOADER_SRC)))
LOADER			:= ../boot/efi/boot/bootx64.efi

HDD				:= ../hdd.img
BUILDER			:= ../tools/image-builder/builder

$(LOADER): $(LOADER_OBJ)
	mkdir -p $(dir $@)
	$(EFI_LD) $(EFI_LDFLAGS) $^
	rm -f $(LOADER:.efi=.lib)

install: $(LOADER)
	mmd -i $(HDD).0 ::/efi -D s || true
	mmd -i $(HDD).0 ::/efi/boot -D s || true
	mcopy -i $(HDD).0 ../boot/efi/boot/bootx64.efi ::/efi/boot/bootx64.efi -D o
	$(BUILDER) ../vineyard/hdd.yaml -m 0 --vmdk --vdi | bash

bin/obj/%.c.o: %.c
	mkdir -p $(dir bin/obj/$*)
ifndef USE_GCC
	$(EFI_CC) $(EFI_CFLAGS) -c $*.c -o $@
else
	$(EFI_CC) $(EFI_CFLAGS) -c $*.c -o $@
endif

bin/obj/%.s.o: %.s
	mkdir -p $(dir bin/obj/$*)
	$(EFI_AS) $(EFI_ASFLAGS) -o $@ $*.s

clean:
	rm -f $(LOADER) $(LOADER_OBJ)
