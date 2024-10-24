.intel_syntax noprefix

.section .text

.global _start
_start:
    push rbp
    mov rbp, rsp
    call main
    mov rdi, rax
    mov rax, 60
    syscall
