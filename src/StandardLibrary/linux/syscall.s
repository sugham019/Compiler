.intel_syntax noprefix

.section .text

.global sys_write
sys_write:
    mov rax, 1
    syscall
    ret

.global sys_read
sys_read:
    mov rax, 0
    syscall
    ret