project('loader', 'c')

warning_level = 3
strip = false
c_std = 'gnu11'

include_dir = include_directories('include')

yasm = find_program('yasm', required: true)

asm_gen = generator(yasm, output: '@BASENAME@.o', arguments: meson.get_cross_property('asm_args') + ['@INPUT@', '-o', '@OUTPUT@'])
asm_src = files('cpu/msr.asm', 'cpu/cr.asm', 'elf/elf.asm')
asm_obj = []

foreach f : asm_src
	asm_obj += asm_gen.process(f)
endforeach

c_src = files(
	'boot/framebuffer.c',
	'boot/image.c',
	'boot/memory_map.c',
	'boot/rsdp.c',
	'elf/elf.c',
	'fs/open.c',
	'fs/read.c',
	'fs/utils.c',
	'lib/stdlib/free.c',
	'lib/stdlib/malloc.c',
	'lib/string/memcmp.c',
	'lib/string/memcpy.c',
	'lib/string/memset.c',
	'lib/string/strlen.c',
	'lib/string/strncmp.c',
	'lib/vcbprintf.c',
	'loader.c',
	'loader/kernel.c',
	'loader/paging.c',
	'loader/parse.c',
	'net/bufpool.c',
	'net/eth.c',
	'net/icmp6.c',
	'net/ip6.c',
	'net/net.c',
	'net/netboot.c',
	'net/udp6.c',
	'net/util.c',
	'print.c',
	'uefi/device-path.c',
	'uefi/driver.c',
	'uefi/error.c',
	'uefi/guid.c',
	'uefi/init.c',
	'uefi/io.c',
	'uefi/ucs2.c',
	'uefi/util.c',
)

c_args = ['-target', 'x86_64-pc-win32-coff',
'-Wbad-function-cast',
'-Wconversion',
'-Wformat=2',
'-Wformat-security',
'-Winit-self',
'-Wparentheses',
'-Winline',
'-Wmissing-braces',
'-Wmissing-declarations',
'-Wmissing-field-initializers',
'-Wmissing-prototypes',
'-Wnested-externs',
'-Wpointer-arith',
'-Wredundant-decls',
'-Wshadow',
'-Wstrict-prototypes',
'-Wswitch-default',
'-Wswitch-enum',
'-Wuninitialized',
'-Wunreachable-code',
'-Wunused',
'-Wwrite-strings',
'-Wno-unreachable-code',
'-mno-red-zone',
'-ffreestanding',
'-fno-stack-protector',
'-mno-mmx',
'-mno-sse',
'-mno-sse2',
'-DVINEYARD',
'-DSTACK=0xFFFFFE0000000000',
'-DSTACK_SIZE=0x21000',
'-DVINEYARD_LOADER', ]

# we need this fake library to compile our C sources
fake_lib = static_library('fakeloader', c_src, c_args: c_args, include_directories: include_dir, pic: false)

# specifying a linker in a cross-file doesn't really work here
custom_target('bootx64.efi',
	output: 'bootx64.efi',
	input: [fake_lib.extract_all_objects(), asm_obj],
	command: [
		'lld-link',
		meson.get_cross_property('ld_args'),
		'@INPUT@',
		'-out:@OUTPUT@'
	],
	install: true, install_dir: '')
