[bits 32]

global _start
extern kernel_main

section .text
_start:
    cli
    mov esp, stack_top
    push dword 0x8000
    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
