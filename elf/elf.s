; void elf_jump_to_kernel(info_t *info, elf64_addr_t entry);
[global elf_jump_to_kernel]
elf_jump_to_kernel:
	mov cr3, rdx
	mov rsp, (STACK + STACK_SIZE)
	mov rbp, rsp
	jmp rsi
