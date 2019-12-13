[global elf_jump_to_kernel]
; void elf_jump_to_kernel(efi_handle handle, efi_system_table *st, uintptr_t kernel, elf64_addr_t entry);
elf_jump_to_kernel:
	mov rdi, rcx
	mov rsi, rdx
	mov rdx, r8
	mov rbp, (STACK + STACK_SIZE)
	mov rsp, rbp
	jmp r9

; SysV: rdi, rsi, rdx
; MS ABI: rcx, rdx, r8, r9
