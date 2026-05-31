[bits 32]
[org 0x10000]

jmp kernel_entry

%include "kernel/include/font.inc"
%include "kernel/drivers/serial.inc"
%include "kernel/drivers/vga.inc"
%include "kernel/assets/icons.inc"

APP_DESKTOP  equ 0
APP_FILE     equ 1
APP_WEB      equ 2
APP_ACTIVITY equ 3
APP_PAINT    equ 4

TARGET_NONE         equ 0
TARGET_ICON_FILE    equ 1
TARGET_ICON_WEB     equ 2
TARGET_ICON_ACTIVITY equ 3
TARGET_ICON_PAINT   equ 4
TARGET_START        equ 5
TARGET_MENU_FILE    equ 6
TARGET_MENU_WEB     equ 7
TARGET_MENU_ACTIVITY equ 8
TARGET_MENU_PAINT   equ 9

KEY_ESC      equ 0x01
KEY_1        equ 0x02
KEY_2        equ 0x03
KEY_3        equ 0x04
KEY_4        equ 0x05
KEY_TAB      equ 0x0F
KEY_ENTER    equ 0x1C
KEY_A        equ 0x1E
KEY_C        equ 0x2E
KEY_D        equ 0x20
KEY_I        equ 0x17
KEY_J        equ 0x24
KEY_K        equ 0x25
KEY_L        equ 0x26
KEY_S        equ 0x1F
KEY_W        equ 0x11
KEY_SPACE    equ 0x39

CURSOR_STEP  equ 8

kernel_entry:
    cli
    mov esp, stack_top
    call serial_init
    call set_palette
    call boot_sequence
    call init_state
    call render
    mov esi, msg_booted
    call serial_write_string
    call debug_write_string

main_loop:
    inc dword [loop_counter]
    call poll_key
    test al, al
    jz .refresh
    call handle_key
.refresh:
    cmp byte [current_screen], APP_ACTIVITY
    jne .idle
    mov eax, [loop_counter]
    and eax, 0x2FFF
    jnz .idle
    call render
.idle:
    jmp main_loop

poll_key:
    in al, 0x64
    test al, 0x01
    jz .none
    in al, 0x60
    cmp al, 0xE0
    je .none
    test al, 0x80
    jnz .none
    ret
.none:
    xor al, al
    ret

handle_key:
    cmp byte [current_screen], APP_PAINT
    je paint_handle_key

    cmp al, KEY_ESC
    jne .not_escape
    cmp byte [current_screen], APP_DESKTOP
    jne .close_window
    mov byte [start_open], 0
    call render
    ret
.close_window:
    mov byte [current_screen], APP_DESKTOP
    mov byte [start_open], 0
    call render
    ret
.not_escape:
    cmp byte [current_screen], APP_DESKTOP
    jne .app_shortcuts
    call handle_desktop_key
    ret
.app_shortcuts:
    cmp al, KEY_1
    je .open_file
    cmp al, KEY_2
    je .open_web
    cmp al, KEY_3
    je .open_activity
    cmp al, KEY_4
    je .open_paint
    ret
.open_file:
    mov eax, APP_FILE
    call open_app
    ret
.open_web:
    mov eax, APP_WEB
    call open_app
    ret
.open_activity:
    mov eax, APP_ACTIVITY
    call open_app
    ret
.open_paint:
    mov eax, APP_PAINT
    call open_app
    ret

handle_desktop_key:
    cmp al, KEY_I
    je .move_up
    cmp al, KEY_K
    je .move_down
    cmp al, KEY_J
    je .move_left
    cmp al, KEY_L
    je .move_right
    cmp al, KEY_TAB
    je .cycle_icons
    cmp al, KEY_ENTER
    je .activate
    cmp al, KEY_S
    je .toggle_start
    cmp al, KEY_1
    je .open_file
    cmp al, KEY_2
    je .open_web
    cmp al, KEY_3
    je .open_activity
    cmp al, KEY_4
    je .open_paint
    ret
.move_up:
    cmp byte [pointer_y], 4
    jbe .rerender
    sub byte [pointer_y], CURSOR_STEP
    jmp .rerender
.move_down:
    cmp byte [pointer_y], 180
    jae .rerender
    add byte [pointer_y], CURSOR_STEP
    jmp .rerender
.move_left:
    cmp byte [pointer_x], 4
    jbe .rerender
    sub byte [pointer_x], CURSOR_STEP
    jmp .rerender
.move_right:
    cmp byte [pointer_x], 240
    jae .rerender
    add byte [pointer_x], CURSOR_STEP
    jmp .rerender
.cycle_icons:
    inc byte [selected_icon]
    cmp byte [selected_icon], 4
    jb .move_pointer
    mov byte [selected_icon], 0
.move_pointer:
    movzx eax, byte [selected_icon]
    mov bl, [icon_pointer_x + eax]
    mov [pointer_x], bl
    mov bl, [icon_pointer_y + eax]
    mov [pointer_y], bl
    jmp .rerender
.activate:
    call desktop_activate
    ret
.toggle_start:
    cmp byte [start_open], 0
    je .open_start
    mov byte [start_open], 0
    jmp .rerender
.open_start:
    mov byte [start_open], 1
    jmp .rerender
.open_file:
    mov eax, APP_FILE
    call open_app
    ret
.open_web:
    mov eax, APP_WEB
    call open_app
    ret
.open_activity:
    mov eax, APP_ACTIVITY
    call open_app
    ret
.open_paint:
    mov eax, APP_PAINT
    call open_app
    ret
.rerender:
    call render
    ret

desktop_activate:
    call update_hover_target
    mov al, [hover_target]
    cmp al, TARGET_ICON_FILE
    je .file
    cmp al, TARGET_ICON_WEB
    je .web
    cmp al, TARGET_ICON_ACTIVITY
    je .activity
    cmp al, TARGET_ICON_PAINT
    je .paint
    cmp al, TARGET_START
    je .toggle_start
    cmp al, TARGET_MENU_FILE
    je .file
    cmp al, TARGET_MENU_WEB
    je .web
    cmp al, TARGET_MENU_ACTIVITY
    je .activity
    cmp al, TARGET_MENU_PAINT
    je .paint
    ret
.toggle_start:
    cmp byte [start_open], 0
    je .open_start
    mov byte [start_open], 0
    call render
    ret
.open_start:
    mov byte [start_open], 1
    call render
    ret
.file:
    mov eax, APP_FILE
    call open_app
    ret
.web:
    mov eax, APP_WEB
    call open_app
    ret
.activity:
    mov eax, APP_ACTIVITY
    call open_app
    ret
.paint:
    mov eax, APP_PAINT
    call open_app
    ret

paint_handle_key:
    cmp al, KEY_ESC
    je .exit_paint
    cmp al, KEY_TAB
    je .cycle_color
    cmp al, KEY_SPACE
    je .paint_pixel
    cmp al, KEY_C
    je .clear_canvas
    cmp al, KEY_W
    je .move_up
    cmp al, KEY_A
    je .move_left
    cmp al, KEY_S
    je .move_down
    cmp al, KEY_D
    je .move_right
    ret
.exit_paint:
    mov byte [current_screen], APP_DESKTOP
    call render
    ret
.cycle_color:
    inc byte [paint_color]
    cmp byte [paint_color], 8
    jb .redraw
    mov byte [paint_color], 0
    jmp .redraw
.paint_pixel:
    movzx eax, byte [paint_cursor_y]
    imul eax, 20
    movzx ebx, byte [paint_cursor_x]
    add eax, ebx
    movzx ebx, byte [paint_color]
    mov bl, [paint_colors + ebx]
    mov [paint_canvas + eax], bl
    jmp .redraw
.clear_canvas:
    lea edi, [paint_canvas]
    mov ecx, 20 * 14
    xor eax, eax
    rep stosb
    jmp .redraw
.move_up:
    cmp byte [paint_cursor_y], 0
    je .redraw
    dec byte [paint_cursor_y]
    jmp .redraw
.move_left:
    cmp byte [paint_cursor_x], 0
    je .redraw
    dec byte [paint_cursor_x]
    jmp .redraw
.move_down:
    cmp byte [paint_cursor_y], 13
    jae .redraw
    inc byte [paint_cursor_y]
    jmp .redraw
.move_right:
    cmp byte [paint_cursor_x], 19
    jae .redraw
    inc byte [paint_cursor_x]
.redraw:
    call render
    ret

open_app:
    mov [current_screen], al
    mov byte [start_open], 0
    call render
    ret

init_state:
    mov byte [current_screen], APP_DESKTOP
    mov byte [selected_icon], 0
    mov byte [start_open], 0
    mov byte [pointer_x], 26
    mov byte [pointer_y], 32
    mov byte [hover_target], TARGET_ICON_FILE
    mov byte [paint_cursor_x], 0
    mov byte [paint_cursor_y], 0
    mov byte [paint_color], 0
    lea edi, [paint_canvas]
    mov ecx, 20 * 14
    xor eax, eax
    rep stosb
    ret

boot_sequence:
    call clear_screen
    mov eax, 0
    mov ebx, 0
    mov ecx, 320
    mov edx, 156
    mov esi, boot_log_screen
    call draw_raw_sprite
    mov byte [boot_stage], 0
    call draw_boot_status
    mov ecx, 36000000
    call delay_loop

    mov byte [boot_stage], 1
    call draw_boot_frame
    mov ecx, 22000000
    call delay_loop

    mov byte [boot_stage], 2
    call draw_boot_frame
    mov ecx, 22000000
    call delay_loop

    mov byte [boot_stage], 3
    call draw_boot_frame
    mov ecx, 22000000
    call delay_loop

    mov byte [boot_stage], 4
    call draw_boot_frame
    mov ecx, 18000000
    call delay_loop
    ret

draw_boot_frame:
    call clear_screen
    mov eax, 0
    mov ebx, 0
    mov ecx, 320
    mov edx, 156
    mov esi, boot_screen
    call draw_raw_sprite
    call draw_boot_status
    ret

draw_boot_status:
    mov eax, 0
    mov ebx, 156
    mov ecx, 320
    mov edx, 44
    mov byte [temp_color], COLOR_BLACK
    call fill_rect

    mov eax, 14
    mov ebx, 164
    mov esi, boot_status_title
    mov byte [temp_color], COLOR_TEXT_LIGHT
    call draw_text_line

    movzx eax, byte [boot_stage]
    cmp eax, 0
    jne .stage1
    mov esi, boot_status_log
    jmp .line1
.stage1:
    mov esi, boot_status_1
.line1:
    mov eax, 14
    mov ebx, 176
    mov byte [temp_color], COLOR_STATUS_TEXT
    call draw_text_line

    mov eax, 14
    mov ebx, 186
    mov esi, boot_status_2
    mov byte [temp_color], COLOR_STATUS_TEXT
    cmp byte [boot_stage], 2
    jb .line2
    mov byte [temp_color], COLOR_TEXT_LIGHT
.line2:
    call draw_text_line

    mov eax, 154
    mov ebx, 176
    mov esi, boot_status_3
    mov byte [temp_color], COLOR_STATUS_TEXT
    cmp byte [boot_stage], 3
    jb .line3
    mov byte [temp_color], COLOR_TEXT_LIGHT
.line3:
    call draw_text_line

    mov eax, 154
    mov ebx, 186
    mov esi, boot_status_4
    mov byte [temp_color], COLOR_STATUS_TEXT
    cmp byte [boot_stage], 4
    jb .line4
    mov byte [temp_color], COLOR_TEXT_LIGHT
.line4:
    call draw_text_line
    ret

delay_loop:
    push eax
.loop:
    dec ecx
    jnz .loop
    pop eax
    ret

update_hover_target:
    mov byte [hover_target], TARGET_NONE
    cmp byte [current_screen], APP_DESKTOP
    jne .done

    mov al, [pointer_x]
    cmp al, 8
    jb .check_start
    cmp al, 48
    ja .check_start
    mov al, [pointer_y]
    cmp al, 18
    jb .check_start
    cmp al, 74
    jbe .file
    cmp al, 90
    jb .check_start
    cmp al, 146
    jbe .activity
    jmp .check_start
.file:
    mov byte [hover_target], TARGET_ICON_FILE
    jmp .done
.activity:
    mov byte [hover_target], TARGET_ICON_ACTIVITY
    jmp .done

.check_start:
    mov al, [pointer_x]
    cmp al, 8
    jb .check_right_icons
    cmp al, 74
    ja .check_right_icons
    mov al, [pointer_y]
    cmp al, 176
    jb .check_right_icons
    cmp al, 196
    jbe .start
.check_right_icons:
    mov al, [pointer_x]
    cmp al, 84
    jb .check_menu
    cmp al, 126
    ja .check_menu
    mov al, [pointer_y]
    cmp al, 18
    jb .check_menu
    cmp al, 74
    jbe .web
    cmp al, 90
    jb .check_menu
    cmp al, 146
    jbe .paint
    jmp .check_menu
.web:
    mov byte [hover_target], TARGET_ICON_WEB
    jmp .done
.paint:
    mov byte [hover_target], TARGET_ICON_PAINT
    jmp .done
.start:
    mov byte [hover_target], TARGET_START
    jmp .done

.check_menu:
    cmp byte [start_open], 1
    jne .done
    mov al, [pointer_x]
    cmp al, 40
    jb .done
    cmp al, 154
    ja .done
    mov al, [pointer_y]
    cmp al, 100
    jb .done
    cmp al, 116
    jb .menu_file
    cmp al, 132
    jb .menu_web
    cmp al, 148
    jb .menu_activity
    cmp al, 164
    jb .menu_paint
    jmp .done
.menu_file:
    mov byte [hover_target], TARGET_MENU_FILE
    jmp .done
.menu_web:
    mov byte [hover_target], TARGET_MENU_WEB
    jmp .done
.menu_activity:
    mov byte [hover_target], TARGET_MENU_ACTIVITY
    jmp .done
.menu_paint:
    mov byte [hover_target], TARGET_MENU_PAINT
.done:
    ret

render:
    call update_hover_target
    call clear_screen
    call draw_desktop

    cmp byte [current_screen], APP_FILE
    je draw_file_app
    cmp byte [current_screen], APP_WEB
    je draw_web_app
    cmp byte [current_screen], APP_ACTIVITY
    je draw_activity_app
    cmp byte [current_screen], APP_PAINT
    je draw_paint_app

.desktop_overlays:
    cmp byte [start_open], 1
    jne .cursor
    call draw_start_menu
.cursor:
    call draw_pointer
    ret

draw_desktop:
    mov eax, 0
    mov ebx, 0
    mov ecx, SCREEN_WIDTH
    mov edx, SCREEN_HEIGHT
    mov byte [temp_color], COLOR_DESKTOP
    call fill_rect

    mov eax, 10
    mov ebx, 8
    mov esi, desktop_title
    mov byte [temp_color], COLOR_TEXT_LIGHT
    call draw_text_line

    mov eax, 10
    mov ebx, 18
    mov esi, desktop_subtitle
    mov byte [temp_color], COLOR_STATUS_TEXT
    call draw_text_line

    call draw_icons
    call draw_taskbar
    ret

draw_icons:
    xor ecx, ecx
.loop:
    cmp ecx, 4
    jae .done
    push ecx
    mov al, [hover_target]
    cmp al, cl
    jne .no_highlight
    inc al
.no_highlight:
    mov al, [hover_target]
    cmp al, cl
    jne .maybe_selected
.maybe_selected:
    cmp cl, [selected_icon]
    jne .draw_icon
    mov eax, [icon_frame_x + ecx * 4]
    mov ebx, [icon_frame_y + ecx * 4]
    mov ecx, 44
    mov edx, 44
    call draw_panel
    pop ecx
    push ecx
.draw_icon:
    mov eax, [icon_pos_x + ecx * 4]
    mov ebx, [icon_pos_y + ecx * 4]
    mov esi, [icon_ptrs + ecx * 4]
    call draw_icon_raw

    mov eax, [icon_text_x + ecx * 4]
    mov ebx, [icon_text_y1 + ecx * 4]
    mov esi, [icon_label_1 + ecx * 4]
    mov byte [temp_color], COLOR_TEXT_LIGHT
    call draw_text_line
    mov eax, [icon_text_x + ecx * 4]
    mov ebx, [icon_text_y2 + ecx * 4]
    mov esi, [icon_label_2 + ecx * 4]
    mov byte [temp_color], COLOR_TEXT_LIGHT
    call draw_text_line
    pop ecx
    inc ecx
    jmp .loop
.done:
    ret

draw_taskbar:
    mov eax, 0
    mov ebx, TASKBAR_Y
    mov ecx, SCREEN_WIDTH
    mov edx, 22
    call draw_panel

    mov eax, 6
    mov ebx, 181
    mov ecx, 60
    mov edx, 15
    call draw_panel

    cmp byte [start_open], 1
    jne .normal_start
    mov eax, 8
    mov ebx, 183
    mov ecx, 56
    mov edx, 11
    mov byte [temp_color], COLOR_START_GREEN
    call fill_rect
.normal_start:
    mov eax, 14
    mov ebx, 185
    mov esi, start_text
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line

    mov eax, 84
    mov ebx, 185
    mov esi, taskbar_hint
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line
    ret

draw_window_frame:
    ; esi=title
    mov eax, 34
    mov ebx, 22
    mov ecx, 252
    mov edx, 148
    call draw_panel

    mov eax, 39
    mov ebx, 27
    mov ecx, 242
    mov edx, 16
    mov byte [temp_color], COLOR_BLUE
    call fill_rect

    mov eax, 41
    mov ebx, 45
    mov ecx, 238
    mov edx, 120
    mov byte [temp_color], COLOR_WINDOW_FILL
    call fill_rect

    mov eax, 46
    mov ebx, 31
    mov byte [temp_color], COLOR_TEXT_LIGHT
    call draw_text_line

    mov eax, 260
    mov ebx, 29
    mov ecx, 15
    mov edx, 11
    call draw_panel
    mov eax, 264
    mov ebx, 31
    mov esi, close_text
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line
    ret

draw_file_app:
    mov esi, title_file
    call draw_window_frame
    mov eax, 48
    mov ebx, 52
    mov ecx, 78
    mov edx, 108
    call draw_panel
    mov eax, 138
    mov ebx, 52
    mov ecx, 136
    mov edx, 108
    call draw_panel
    mov eax, 52
    mov ebx, 58
    mov esi, file_tree
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line
    mov eax, 146
    mov ebx, 58
    mov esi, file_info
    call draw_text_line
    ret

draw_web_app:
    mov esi, title_web
    call draw_window_frame
    mov eax, 48
    mov ebx, 52
    mov ecx, 224
    mov edx, 14
    mov byte [temp_color], COLOR_WHITE
    call fill_rect
    mov eax, 54
    mov ebx, 55
    mov esi, web_address
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line
    mov eax, 52
    mov ebx, 76
    mov esi, web_body
    call draw_text_line
    ret

draw_activity_app:
    mov esi, title_activity
    call draw_window_frame
    mov eax, 50
    mov ebx, 56
    mov esi, activity_line_1
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line
    mov eax, 148
    mov ebx, 56
    call write_loop_count
    mov esi, decimal_buffer
    call draw_text_line

    mov eax, 50
    mov ebx, 68
    mov esi, activity_line_2
    call draw_text_line
    mov eax, 148
    mov ebx, 68
    movzx eax, byte [current_screen]
    call write_open_app_name
    mov esi, open_app_buffer
    call draw_text_line

    mov eax, 50
    mov ebx, 86
    mov esi, activity_block
    call draw_text_line
    ret

draw_paint_app:
    mov esi, title_paint
    call draw_window_frame
    mov eax, 50
    mov ebx, 54
    mov esi, paint_help_1
    mov byte [temp_color], COLOR_TEXT_DARK
    call draw_text_line
    mov eax, 50
    mov ebx, 66
    mov esi, paint_help_2
    call draw_text_line
    call draw_paint_canvas
    ret

draw_paint_canvas:
    xor edi, edi
.row:
    cmp edi, 14
    jae .done
    xor esi, esi
.col:
    cmp esi, 20
    jae .next_row
    mov eax, edi
    imul eax, 20
    add eax, esi
    mov al, [paint_canvas + eax]
    test al, al
    jnz .have_color
    mov al, COLOR_WHITE
.have_color:
    mov eax, 58
    mov ebx, 88
    mov ecx, esi
    imul ecx, 8
    add eax, ecx
    mov ecx, edi
    imul ecx, 8
    add ebx, ecx
    mov ecx, 7
    mov edx, 7
    mov [temp_color], al
    call fill_rect

    movzx eax, byte [paint_cursor_x]
    cmp eax, esi
    jne .not_cursor
    movzx eax, byte [paint_cursor_y]
    cmp eax, edi
    jne .not_cursor
    mov eax, 57
    mov ebx, 87
    mov ecx, esi
    imul ecx, 8
    add eax, ecx
    mov ecx, edi
    imul ecx, 8
    add ebx, ecx
    mov ecx, 9
    mov edx, 9
    call draw_panel
.not_cursor:
    inc esi
    jmp .col
.next_row:
    inc edi
    jmp .row
.done:
    mov eax, 232
    mov ebx, 88
    mov ecx, 24
    mov edx, 24
    call draw_panel
    mov eax, 237
    mov ebx, 93
    mov ecx, 14
    mov edx, 14
    movzx esi, byte [paint_color]
    mov al, [paint_colors + esi]
    mov [temp_color], al
    call fill_rect
    ret

draw_start_menu:
    mov eax, 8
    mov ebx, 92
    mov ecx, 148
    mov edx, 84
    call draw_panel

    mov eax, 13
    mov ebx, 98
    mov ecx, 22
    mov edx, 72
    mov byte [temp_color], COLOR_BLUE
    call fill_rect

    mov eax, 16
    mov ebx, 104
    mov esi, start_brand_1
    mov byte [temp_color], COLOR_TEXT_LIGHT
    call draw_text_line
    mov eax, 16
    mov ebx, 116
    mov esi, start_brand_2
    call draw_text_line

    call draw_start_item_file
    call draw_start_item_web
    call draw_start_item_activity
    call draw_start_item_paint
    ret

draw_start_item_file:
    mov eax, 42
    mov ebx, 100
    mov esi, start_item_1
    mov bl, TARGET_MENU_FILE
    jmp draw_start_item_common

draw_start_item_web:
    mov eax, 42
    mov ebx, 116
    mov esi, start_item_2
    mov bl, TARGET_MENU_WEB
    jmp draw_start_item_common

draw_start_item_activity:
    mov eax, 42
    mov ebx, 132
    mov esi, start_item_3
    mov bl, TARGET_MENU_ACTIVITY
    jmp draw_start_item_common

draw_start_item_paint:
    mov eax, 42
    mov ebx, 148
    mov esi, start_item_4
    mov bl, TARGET_MENU_PAINT

draw_start_item_common:
    push eax
    push ebx
    push esi
    mov al, [hover_target]
    cmp al, bl
    jne .plain
    pop esi
    pop ebx
    pop eax
    push esi
    push ebx
    push eax
    mov ecx, 106
    mov edx, 14
    mov byte [temp_color], COLOR_BLUE
    call fill_rect
    mov byte [temp_color], COLOR_TEXT_LIGHT
    jmp .text
.plain:
    mov byte [temp_color], COLOR_TEXT_DARK
.text:
    pop eax
    pop ebx
    pop esi
    call draw_text_line
    ret

draw_pointer:
    mov eax, [pointer_x_draw]
    mov ebx, [pointer_y_draw]
    mov ecx, 16
    mov edx, 16
    mov esi, cursor_icon
    cmp byte [hover_target], TARGET_NONE
    je .draw
    mov esi, cursor_selected_icon
.draw:
    movzx eax, byte [pointer_x]
    movzx ebx, byte [pointer_y]
    call draw_raw_sprite
    ret

write_loop_count:
    mov eax, [loop_counter]
    jmp u32_to_decimal

write_open_app_name:
    cmp eax, APP_FILE
    je .file
    cmp eax, APP_WEB
    je .web
    cmp eax, APP_ACTIVITY
    je .activity
    cmp eax, APP_PAINT
    je .paint
    mov esi, desktop_name
    jmp .copy
.file:
    mov esi, file_name
    jmp .copy
.web:
    mov esi, web_name
    jmp .copy
.activity:
    mov esi, activity_name
    jmp .copy
.paint:
    mov esi, paint_name
.copy:
    lea edi, [open_app_buffer]
.loop:
    lodsb
    stosb
    test al, al
    jnz .loop
    ret

u32_to_decimal:
    lea edi, [decimal_buffer + 15]
    mov byte [edi], 0
    cmp eax, 0
    jne .convert
    dec edi
    mov byte [edi], '0'
    jmp .finish
.convert:
    mov ebx, 10
.next_digit:
    xor edx, edx
    div ebx
    add dl, '0'
    dec edi
    mov [edi], dl
    test eax, eax
    jnz .next_digit
.finish:
    lea esi, [edi]
    lea edi, [decimal_buffer]
.copy:
    lodsb
    stosb
    test al, al
    jnz .copy
    ret

desktop_title db 'COPPEROS',0
desktop_subtitle db 'WINDOWS 95 INSPIRED DESKTOP',0
start_text db 'START',0
taskbar_hint db 'IJKL MOVE  ENTER OPEN  1-4 APPS',0
close_text db 'X',0

title_file db 'FILE EXPLORER',0
title_web db 'INTERNET EXPLORER',0
title_activity db 'ACTIVITY MANAGER',0
title_paint db 'PAINT',0

boot_status_title db 'STARTING COPPEROS',0
boot_status_log db 'READING BOOT LOGO',0
boot_status_1 db 'PREPARING DRIVERS',0
boot_status_2 db 'LOADING APPS',0
boot_status_3 db 'BUILDING USER INTERFACE',0
boot_status_4 db 'FINALIZING DESKTOP',0

file_tree db 'DESKTOP/',10,'BOOT/',10,'KERNEL/',10,'APPS/',10,'ASSETS/',10,'DOCS/',10,'MAKEFILE',0
file_info db 'PROJECT VIEW',10,'FOUR BUILT IN APPS',10,'BOOT ART READY',10,'CURSOR STATES READY',10,'REAL QEMU BOOT IMAGE',0
web_address db 'HTTPS://COPPEROS.LOCAL/',0
web_body db 'INTERNET EXPLORER',10,'NETWORK STACK: NOT YET LIVE',10,'THIS BUILD IS OFFLINE',10,'NEXT STEP: REAL TCP IP',10,'AND A REAL BROWSER ENGINE',0
activity_line_1 db 'FRAME LOOPS:',0
activity_line_2 db 'FOREGROUND APP:',0
activity_block db 'ARCH: 32 BIT FOR NOW',10,'POINTER: KEYBOARD DRIVEN',10,'DISPLAY: 320 X 200',10,'NET: PLANNED NEXT',0
paint_help_1 db 'WASD MOVE  SPACE DRAW  TAB COLOR',0
paint_help_2 db 'C CLEAR  ESC CLOSE',0

start_brand_1 db 'COP',0
start_brand_2 db 'OS',0

icon_file_1 db 'FILE',0
icon_file_2 db 'EXPLORER',0
icon_web_1 db 'INTERNET',0
icon_web_2 db 'EXPLORER',0
icon_activity_1 db 'ACTIVITY',0
icon_activity_2 db 'MANAGER',0
icon_paint_1 db 'PAINT',0
icon_paint_2 db 0

desktop_name db 'DESKTOP',0
file_name db 'FILE EXPLORER',0
web_name db 'INTERNET EXPLORER',0
activity_name db 'ACTIVITY MANAGER',0
paint_name db 'PAINT',0

start_item_1 db 'FILE EXPLORER',0
start_item_2 db 'INTERNET EXPLORER',0
start_item_3 db 'ACTIVITY MANAGER',0
start_item_4 db 'PAINT',0

icon_pos_x dd 14, 92, 14, 92
icon_pos_y dd 26, 26, 98, 98
icon_frame_x dd 8, 86, 8, 86
icon_frame_y dd 22, 22, 94, 94
icon_text_x dd 10, 82, 6, 88
icon_text_y1 dd 60, 60, 132, 132
icon_text_y2 dd 70, 70, 142, 142
icon_ptrs dd filemanager_icon, internetexplorer_icon, activitymanager_icon, paint_icon
icon_label_1 dd icon_file_1, icon_web_1, icon_activity_1, icon_paint_1
icon_label_2 dd icon_file_2, icon_web_2, icon_activity_2, icon_paint_2
icon_pointer_x db 26, 104, 26, 104
icon_pointer_y db 32, 32, 104, 104

msg_booted db 'CopperOS booted',13,10,0

current_screen db 0
selected_icon db 0
start_open db 0
pointer_x db 0
pointer_y db 0
hover_target db 0
boot_stage db 0
paint_cursor_x db 0
paint_cursor_y db 0
paint_color db 0
align 4
loop_counter dd 0
pointer_x_draw dd 0
pointer_y_draw dd 0
decimal_buffer times 16 db 0
open_app_buffer times 32 db 0
paint_canvas times 20 * 14 db 0
paint_colors db COLOR_BLUE, COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_ORANGE, COLOR_CYAN, COLOR_WHITE, COLOR_PANEL_DARK

times 4096 db 0
stack_top:
