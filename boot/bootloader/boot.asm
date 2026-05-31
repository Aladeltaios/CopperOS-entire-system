[bits 16]
[org 0x7C00]

%include "build/kernel_sectors.inc"

KERNEL_LOAD_SEG    equ 0x1000
BOOT_INFO_ADDR     equ 0x8000
MODE_INFO_ADDR     equ 0x8200
VBE_MODE_PREFERRED equ 0x14C
VBE_MODE_1024      equ 0x105
VBE_MODE_800       equ 0x103
VBE_MODE_640       equ 0x101

BOOT_FB_ADDR       equ BOOT_INFO_ADDR + 0
BOOT_PITCH         equ BOOT_INFO_ADDR + 4
BOOT_WIDTH         equ BOOT_INFO_ADDR + 6
BOOT_HEIGHT        equ BOOT_INFO_ADDR + 8
BOOT_BPP           equ BOOT_INFO_ADDR + 10
BOOT_MODE          equ BOOT_INFO_ADDR + 11
start:
    cli
    mov [boot_drive], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    call load_kernel
    mov si, vbe_modes
    call setup_video
    call enable_a20

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:protected_mode

load_kernel:
    mov word [remaining_sectors], KERNEL_SECTORS
    mov word [disk_packet.count], 16
    mov word [disk_packet.offset], 0x0000
    mov word [disk_packet.segment], KERNEL_LOAD_SEG
    mov dword [disk_packet.lba_low], 1
    mov dword [disk_packet.lba_high], 0

.read_next:
    cmp word [remaining_sectors], 0
    je .done

    mov ax, [remaining_sectors]
    cmp ax, 16
    jbe .chunk_ready
    mov ax, 16
.chunk_ready:
    mov [disk_packet.count], ax
    mov si, disk_packet
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jnc .read_ok
    call read_chs_chunk
.read_ok:

    mov ax, [disk_packet.count]
    shl ax, 5
    add word [disk_packet.segment], ax

    mov ax, [disk_packet.count]
    add word [disk_packet.lba_low], ax
    sub word [remaining_sectors], ax
    jmp .read_next

.done:
    ret

read_chs_chunk:
    mov ax, [disk_packet.segment]
    mov [chs_segment], ax
    mov ax, [disk_packet.lba_low]
    mov [chs_lba], ax
    mov ax, [disk_packet.count]
    mov [chs_count], ax

.next_sector:
    cmp word [chs_count], 0
    je .done

    mov ax, [chs_lba]
    xor dx, dx
    mov bx, 18
    div bx
    mov cl, dl
    inc cl

    xor dx, dx
    mov bx, 2
    div bx
    mov ch, al
    mov dh, dl

    mov ax, [chs_segment]
    mov es, ax
    xor bx, bx
    mov ax, 0x0201
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    add word [chs_segment], 32
    inc word [chs_lba]
    dec word [chs_count]
    jmp .next_sector

.done:
    xor ax, ax
    mov es, ax
    ret

setup_video:
.try_next_mode:
    lodsw
    test ax, ax
    jz .fallback
    mov [current_vbe_mode], ax

    mov ax, 0x4F01
    mov cx, [current_vbe_mode]
    xor di, di
    mov es, di
    mov di, MODE_INFO_ADDR
    push si
    int 0x10
    pop si
    cmp ax, 0x004F
    jne .try_next_mode
    cmp byte [MODE_INFO_ADDR + 25], 8
    jne .try_next_mode
    mov ax, 0x4F02
    mov bx, 0x4000
    or bx, [current_vbe_mode]
    push si
    int 0x10
    pop si
    cmp ax, 0x004F
    jne .try_next_mode

    mov eax, [MODE_INFO_ADDR + 40]
    mov [BOOT_FB_ADDR], eax
    mov ax, [MODE_INFO_ADDR + 16]
    mov [BOOT_PITCH], ax
    mov ax, [MODE_INFO_ADDR + 18]
    mov [BOOT_WIDTH], ax
    mov ax, [MODE_INFO_ADDR + 20]
    mov [BOOT_HEIGHT], ax
    mov al, [MODE_INFO_ADDR + 25]
    mov [BOOT_BPP], al
    mov byte [BOOT_MODE], 1
    ret

.fallback:
    mov ax, 0x0013
    int 0x10
    mov dword [BOOT_FB_ADDR], 0x000A0000
    mov word [BOOT_PITCH], 320
    mov word [BOOT_WIDTH], 320
    mov word [BOOT_HEIGHT], 200
    mov byte [BOOT_BPP], 8
    mov byte [BOOT_MODE], 0
    ret

enable_a20:
    in al, 0x92
    or al, 0x02
    out 0x92, al
    ret

disk_error:
    cli
.hang:
    hlt
    jmp .hang

[bits 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00
    mov eax, 0x10000
    jmp eax

align 8
gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

align 4
disk_packet:
    db 0x10
    db 0x00
  .count:
    dw 16
  .offset:
    dw 0x0000
  .segment:
    dw KERNEL_LOAD_SEG
  .lba_low:
    dd 0x00000001
  .lba_high:
    dd 0x00000000

boot_drive db 0
remaining_sectors dw 0
current_vbe_mode dw 0
vbe_modes dw VBE_MODE_PREFERRED, VBE_MODE_1024, VBE_MODE_800, VBE_MODE_640, 0
chs_segment dw 0
chs_lba dw 0
chs_count dw 0

times 510 - ($ - $$) db 0
dw 0xAA55
