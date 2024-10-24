.intel_syntax noprefix

.section .text

.global sys_write
sys_write:
    push rbp
    mov rbp, rsp
    mov rax, 1
    syscall
    pop rbp
    ret

.global sys_read
sys_read:
    push rbp
    mov rbp, rsp
    mov rax, 0
    syscall
    pop rbp
    ret