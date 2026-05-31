FRAMEBUFFER equ 0xA0000
SCREEN_WIDTH equ 320
SCREEN_HEIGHT equ 200
TASKBAR_Y equ 178

; Palette indices within a 6x6x6 color cube.
COLOR_BLACK        equ 0
COLOR_WHITE        equ 215
COLOR_DESKTOP      equ 0
COLOR_TASKBAR      equ 188
COLOR_PANEL_LIGHT  equ 215
COLOR_PANEL_DARK   equ 72
COLOR_PANEL_FILL   equ 194
COLOR_BLUE         equ 110
COLOR_TEXT_DARK    equ 0
COLOR_TEXT_LIGHT   equ 215
COLOR_GREEN        equ 102
COLOR_RED          equ 180
COLOR_YELLOW       equ 210
COLOR_ORANGE       equ 204
COLOR_CYAN         equ 29
COLOR_START_GREEN  equ 100
COLOR_WINDOW_FILL  equ 209
COLOR_STATUS_TEXT  equ 194

set_palette:
    pushad
    mov dx, 0x3C8
    xor al, al
    out dx, al
    inc dx

    xor ebx, ebx
.cube_r:
    xor ecx, ecx
.cube_g:
    xor esi, esi
.cube_b:
    mov eax, ebx
    imul eax, 12
    cmp ebx, 5
    jne .r_ok
    mov al, 63
.r_ok:
    out dx, al

    mov eax, ecx
    imul eax, 12
    cmp ecx, 5
    jne .g_ok
    mov al, 63
.g_ok:
    out dx, al

    mov eax, esi
    imul eax, 12
    cmp esi, 5
    jne .b_ok
    mov al, 63
.b_ok:
    out dx, al

    inc esi
    cmp esi, 6
    jb .cube_b

    inc ecx
    cmp ecx, 6
    jb .cube_g

    inc ebx
    cmp ebx, 6
    jb .cube_r

    xor ecx, ecx
.gray:
    mov eax, ecx
    shl eax, 1
    cmp al, 63
    jbe .gray_ok
    mov al, 63
.gray_ok:
    out dx, al
    out dx, al
    out dx, al
    inc ecx
    cmp ecx, 40
    jb .gray
    popad
    ret

clear_screen:
    pushad
    mov edi, FRAMEBUFFER
    mov al, COLOR_DESKTOP
    mov ecx, SCREEN_WIDTH * SCREEN_HEIGHT
    rep stosb
    popad
    ret

put_pixel:
    ; eax = x, ebx = y, color comes from [temp_color]
    push edi
    imul ebx, SCREEN_WIDTH
    add ebx, eax
    mov edi, FRAMEBUFFER
    add edi, ebx
    mov al, [temp_color]
    mov [edi], al
    pop edi
    ret

fill_rect:
    ; eax=x ebx=y ecx=w edx=h, color comes from [temp_color]
    pushad
    mov [fill_width], ecx
    mov ebp, edx
    mov esi, ebx
.row:
    cmp ebp, 0
    je .done
    mov edi, FRAMEBUFFER
    mov ebx, esi
    imul ebx, SCREEN_WIDTH
    add ebx, eax
    add edi, ebx
    mov ecx, [fill_width]
    mov al, [temp_color]
    rep stosb
    inc esi
    dec ebp
    jmp .row
.done:
    popad
    ret

draw_rect:
    ; eax=x ebx=y ecx=w edx=h, color comes from [temp_color]
    pushad
    mov [fill_width], ecx
    mov [draw_x], eax
    mov [draw_y], ebx
    mov [draw_w], ecx
    mov [draw_h], edx
    mov ecx, [fill_width]
    mov edx, 1
    call fill_rect

    mov eax, [draw_x]
    mov ebx, [draw_y]
    add ebx, [draw_h]
    dec ebx
    mov ecx, [draw_w]
    mov edx, 1
    call fill_rect

    mov eax, [draw_x]
    mov ebx, [draw_y]
    mov ecx, 1
    mov edx, [draw_h]
    call fill_rect

    mov eax, [draw_x]
    add eax, [draw_w]
    dec eax
    mov ebx, [draw_y]
    mov ecx, 1
    mov edx, [draw_h]
    call fill_rect
    popad
    ret

draw_panel:
    ; eax=x ebx=y ecx=w edx=h
    pushad
    mov [draw_x], eax
    mov [draw_y], ebx
    mov [draw_w], ecx
    mov [draw_h], edx

    mov byte [temp_color], COLOR_PANEL_FILL
    mov [fill_width], ecx
    call fill_rect

    mov byte [temp_color], COLOR_PANEL_LIGHT
    mov ecx, [draw_w]
    mov edx, 1
    call fill_rect

    mov ecx, 1
    mov edx, [draw_h]
    call fill_rect

    mov eax, [draw_x]
    mov ebx, [draw_y]
    add ebx, [draw_h]
    dec ebx
    mov ecx, [draw_w]
    mov edx, 1
    mov byte [temp_color], COLOR_PANEL_DARK
    call fill_rect

    mov eax, [draw_x]
    add eax, [draw_w]
    dec eax
    mov ebx, [draw_y]
    mov ecx, 1
    mov edx, [draw_h]
    call fill_rect
    popad
    ret

draw_text:
    ; eax=x ebx=y esi=string, color comes from [temp_color]
    pushad
    mov [text_x], eax
    mov [text_origin_x], eax
.next_char:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    jne .render_char
    mov eax, [text_origin_x]
    mov [text_x], eax
    add ebx, 10
    jmp .next_char
.render_char:
    mov [glyph_char], al
    mov [char_y], ebx
    mov eax, [text_x]
    mov [char_x], eax
    mov al, [glyph_char]
    call draw_char
    add dword [text_x], 7
    jmp .next_char
.done:
    popad
    ret

draw_text_line:
    ; eax=x ebx=y esi=string, color comes from [temp_color]
    mov [text_origin_x], eax
    jmp draw_text

draw_char:
    ; al=char, color comes from [temp_color]
    pushad
    mov [glyph_char], al
    mov edi, glyph_chars
    xor ecx, ecx
.find:
    cmp ecx, glyph_count
    jae .done
    mov al, [edi + ecx]
    cmp al, [glyph_char]
    je .found
    inc ecx
    jmp .find
.found:
    mov esi, glyph_data
    lea esi, [esi + ecx * 8]
    xor ecx, ecx
.row_loop:
    cmp ecx, 8
    je .done
    mov ah, [esi + ecx]
    xor edi, edi
.bit_loop:
    cmp edi, 8
    je .next_row
    test ah, 0x80
    jz .skip
    mov eax, [char_x]
    add eax, edi
    mov ebx, [char_y]
    add ebx, ecx
    call put_pixel
.skip:
    shl ah, 1
    inc edi
    jmp .bit_loop
.next_row:
    inc ecx
    jmp .row_loop
.done:
    popad
    ret

draw_bmp24:
    ; eax=x ebx=y esi=bmp data
    pushad
    mov [bmp_x], eax
    mov [bmp_y], ebx
    mov edi, esi
    mov ebp, [edi + 10]
    add ebp, edi
    mov ecx, [edi + 18]
    mov [bmp_width], ecx
    mov ecx, [edi + 22]
    test ecx, ecx
    jns .positive_height
    neg ecx
.positive_height:
    mov [bmp_height], ecx
    mov eax, [bmp_width]
    lea eax, [eax + eax * 2]
    add eax, 3
    and eax, 0xFFFFFFFC
    mov [bmp_stride], eax
    xor edx, edx
.bmp_row:
    cmp edx, [bmp_height]
    jae .done
    xor ecx, ecx
.bmp_col:
    cmp ecx, [bmp_width]
    jae .next_bmp_row
    push ecx
    push edx
    mov eax, edx
    imul eax, [bmp_stride]
    add eax, ebp
    mov edi, ecx
    lea eax, [eax + ecx * 2]
    add eax, edi
    movzx ecx, byte [eax]
    movzx ebx, byte [eax + 1]
    movzx eax, byte [eax + 2]
    cmp eax, 248
    jb .opaque
    cmp ebx, 248
    jb .opaque
    cmp ecx, 248
    pop edx
    pop ecx
    jae .skip_pixel
.opaque:
    call rgb_to_palette
    mov [temp_color], al
    pop edx
    pop ecx
    mov eax, [bmp_x]
    add eax, ecx
    mov ebx, [bmp_y]
    add ebx, edx
    call put_pixel
.skip_pixel:
    inc ecx
    jmp .bmp_col
.next_bmp_row:
    inc edx
    jmp .bmp_row
.done:
    popad
    ret

draw_icon_raw:
    ; eax=x ebx=y esi=32x32 indexed icon, 255 is transparent
    pushad
    mov [raw_x], eax
    mov [raw_y], ebx
    mov edi, esi
    xor edx, edx
.raw_row:
    cmp edx, 32
    jae .raw_done
    xor ecx, ecx
.raw_col:
    cmp ecx, 32
    jae .raw_next_row
    mov al, [edi]
    inc edi
    cmp al, 255
    je .raw_skip
    mov [temp_color], al
    mov eax, [raw_x]
    add eax, ecx
    mov ebx, [raw_y]
    add ebx, edx
    call put_pixel
.raw_skip:
    inc ecx
    jmp .raw_col
.raw_next_row:
    inc edx
    jmp .raw_row
.raw_done:
    popad
    ret

draw_raw_sprite:
    ; eax=x ebx=y ecx=width edx=height esi=data, 255 is transparent
    pushad
    mov [raw_x], eax
    mov [raw_y], ebx
    mov [raw_w], ecx
    mov [raw_h], edx
    mov edi, esi
    xor edx, edx
.row:
    cmp edx, [raw_h]
    jae .done
    xor ecx, ecx
.col:
    cmp ecx, [raw_w]
    jae .next_row
    mov al, [edi]
    inc edi
    cmp al, 255
    je .skip
    mov [temp_color], al
    mov eax, [raw_x]
    add eax, ecx
    mov ebx, [raw_y]
    add ebx, edx
    call put_pixel
.skip:
    inc ecx
    jmp .col
.next_row:
    inc edx
    jmp .row
.done:
    popad
    ret

rgb_to_palette:
    ; eax=r ebx=g ecx=b, returns al=index
    push edx
    call quantize_eax
    imul eax, eax, 36
    push eax
    mov eax, ebx
    call quantize_eax
    imul eax, eax, 6
    mov edx, eax
    mov eax, ecx
    call quantize_eax
    add eax, edx
    pop edx
    add eax, edx
    pop edx
    ret

quantize_eax:
    cmp eax, 43
    jb .q0
    cmp eax, 94
    jb .q1
    cmp eax, 145
    jb .q2
    cmp eax, 196
    jb .q3
    cmp eax, 247
    jb .q4
    mov eax, 5
    ret
.q0:
    xor eax, eax
    ret
.q1:
    mov eax, 1
    ret
.q2:
    mov eax, 2
    ret
.q3:
    mov eax, 3
    ret
.q4:
    mov eax, 4
    ret

temp_color db 0
align 4
fill_width dd 0
draw_x dd 0
draw_y dd 0
draw_w dd 0
draw_h dd 0
text_x dd 0
text_origin_x dd 0
char_x dd 0
char_y dd 0
glyph_char db 0
bmp_x dd 0
bmp_y dd 0
bmp_width dd 0
bmp_height dd 0
bmp_stride dd 0
raw_x dd 0
raw_y dd 0
raw_w dd 0
raw_h dd 0
