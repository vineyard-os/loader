[global elf_jump_to_kernel]
; void elf_jump_to_kernel(efi_handle handle, efi_system_table *st, uintptr_t kernel, elf64_addr_t entry);
elf_jump_to_kernel:
	mov rax, [rsp + 0x28]
	mov rdi, rcx
	mov rsi, rdx
	mov rdx, r8
	mov rcx, r9
	mov rbp, (STACK + STACK_SIZE)
	mov rsp, rbp
	jmp rax

; SysV: rdi, rsi, rdx, rcx, r8, r9
; MS ABI: rcx, rdx, r8, r9
