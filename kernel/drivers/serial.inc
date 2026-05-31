serial_init:
    mov dx, 0x3F8 + 1
    mov al, 0x00
    out dx, al

    mov dx, 0x3F8 + 3
    mov al, 0x80
    out dx, al

    mov dx, 0x3F8 + 0
    mov al, 0x03
    out dx, al

    mov dx, 0x3F8 + 1
    mov al, 0x00
    out dx, al

    mov dx, 0x3F8 + 3
    mov al, 0x03
    out dx, al

    mov dx, 0x3F8 + 2
    mov al, 0xC7
    out dx, al

    mov dx, 0x3F8 + 4
    mov al, 0x0B
    out dx, al
    ret

debug_write_char:
    mov dx, 0xE9
    mov al, bl
    out dx, al
    ret

debug_write_string:
    push eax
    push ebx
    push esi
.loop:
    lodsb
    test al, al
    jz .done
    mov bl, al
    call debug_write_char
    jmp .loop
.done:
    pop esi
    pop ebx
    pop eax
    ret

serial_write_char:
    push edx
.wait:
    mov dx, 0x3F8 + 5
    in al, dx
    test al, 0x20
    jz .wait
    mov dx, 0x3F8
    mov al, bl
    out dx, al
    pop edx
    ret

serial_write_string:
    push eax
    push ebx
    push esi
.loop:
    lodsb
    test al, al
    jz .done
    mov bl, al
    call serial_write_char
    jmp .loop
.done:
    pop esi
    pop ebx
    pop eax
    ret
