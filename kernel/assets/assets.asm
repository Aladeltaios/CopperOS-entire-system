[bits 32]

global filemanager_icon
global internetexplorer_icon
global activitymanager_icon
global paint_icon
global systemsettings_icon
global systemupdates_icon
global calculator_icon
global txteditor_icon
global boot_log_screen
global boot_screen
global cursor_icon
global cursor_selected_icon

section .rodata
align 16
filemanager_icon:
    incbin "build/assets/filemanager_48.raw"

align 16
internetexplorer_icon:
    incbin "build/assets/internetexplorer_48.raw"

align 16
activitymanager_icon:
    incbin "build/assets/activitymanager_48.raw"

align 16
paint_icon:
    incbin "build/assets/paint_48.raw"

align 16
systemsettings_icon:
    incbin "build/assets/systemsettings_48.raw"

align 16
systemupdates_icon:
    incbin "build/assets/systemupdates_48.raw"

align 16
calculator_icon:
    incbin "build/assets/calculator_48.raw"

align 16
txteditor_icon:
    incbin "build/assets/txteditor_48.raw"

align 16
boot_log_screen:
    incbin "build/assets/bootupscreenlog_640x312.raw"

align 16
boot_screen:
    incbin "build/assets/bootupscreen_640x312.raw"

align 16
cursor_icon:
    incbin "build/assets/cursor_20.raw"

align 16
cursor_selected_icon:
    incbin "build/assets/cursorselected_20.raw"

global redsnake_icon
align 16
redsnake_icon:
    incbin "build/assets/redsnake_48.raw"

global cstore_icon
align 16
cstore_icon:
    incbin "build/assets/cstore_48.raw"



