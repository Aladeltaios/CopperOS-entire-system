#include "bootinfo.h"
#include "types.h"

extern const u8 filemanager_icon[];
extern const u8 internetexplorer_icon[];
extern const u8 activitymanager_icon[];
extern const u8 paint_icon[];
extern const u8 systemsettings_icon[];
extern const u8 systemupdates_icon[];
extern const u8 calculator_icon[];
extern const u8 boot_log_screen[];
extern const u8 boot_screen[];
extern const u8 cursor_icon[];
extern const u8 cursor_selected_icon[];

static const char *const version_name = "1.0 Copperhead";

enum {
    APP_DESKTOP = 0,
    APP_FILE = 1,
    APP_WEB = 2,
    APP_ACTIVITY = 3,
    APP_PAINT = 4,
    APP_SETUP = 5,
    APP_SETTINGS = 6,
    APP_UPDATES = 7,
    APP_CALCULATOR = 8,
};

enum {
    TARGET_NONE = 0,
    TARGET_ICON_FILE = 1,
    TARGET_ICON_WEB = 2,
    TARGET_ICON_ACTIVITY = 3,
    TARGET_ICON_PAINT = 4,
    TARGET_ICON_SETTINGS = 5,
    TARGET_ICON_UPDATES = 6,
    TARGET_ICON_CALCULATOR = 7,
    TARGET_START = 8,
    TARGET_MENU_FILE = 9,
    TARGET_MENU_WEB = 10,
    TARGET_MENU_ACTIVITY = 11,
    TARGET_MENU_PAINT = 12,
    TARGET_MENU_SETTINGS = 13,
    TARGET_MENU_UPDATES = 14,
    TARGET_MENU_CALCULATOR = 15,
    TARGET_MENU_SHUTDOWN = 16,
    TARGET_MENU_RESTART = 17,
    TARGET_MENU_SUSPEND = 18,
    TARGET_WINDOW_CLOSE = 19,
};

enum {
    KEY_ESC = 0x01,
    KEY_1 = 0x02,
    KEY_2 = 0x03,
    KEY_3 = 0x04,
    KEY_4 = 0x05,
    KEY_5 = 0x06,
    KEY_6 = 0x07,
    KEY_7 = 0x08,
    KEY_0 = 0x0B,
    KEY_TAB = 0x0F,
    KEY_ENTER = 0x1C,
    KEY_BACKSPACE = 0x0E,
    KEY_A = 0x1E,
    KEY_B = 0x30,
    KEY_C = 0x2E,
    KEY_D = 0x20,
    KEY_I = 0x17,
    KEY_J = 0x24,
    KEY_K = 0x25,
    KEY_L = 0x26,
    KEY_M = 0x32,
    KEY_N = 0x31,
    KEY_S = 0x1F,
    KEY_W = 0x11,
    KEY_X = 0x2D,
    KEY_SPACE = 0x39,
};

enum {
    COLOR_BLACK = 0,
    COLOR_WHITE = 215,
    COLOR_DESKTOP = 0,
    COLOR_PANEL_LIGHT = 215,
    COLOR_PANEL_DARK = 72,
    COLOR_PANEL_FILL = 194,
    COLOR_BLUE = 110,
    COLOR_TEXT_DARK = 0,
    COLOR_TEXT_LIGHT = 215,
    COLOR_GREEN = 102,
    COLOR_RED = 180,
    COLOR_YELLOW = 210,
    COLOR_ORANGE = 204,
    COLOR_CYAN = 29,
    COLOR_START_GREEN = 100,
    COLOR_WINDOW_FILL = 209,
    COLOR_STATUS_TEXT = 194,
};

enum {
  DIRTY_NONE = 0,
  DIRTY_POINTER = 1,
  DIRTY_STARTMENU = 2,
  DIRTY_APP = 4,
  DIRTY_FULL = 0xFF
};

static u8 dirty_flags;
static s32 prev_pointer_x;
static s32 prev_pointer_y;

typedef struct Rect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
} Rect;

static BootInfo *g_boot;
static volatile u8 *g_fb;
static u16 g_width;
static u16 g_height;
static u16 g_pitch;

static u8 current_screen;
static u8 selected_icon;
static u8 start_open;
static s32 pointer_x;
static s32 pointer_y;
static u8 hover_target;
static u32 loop_counter;
static u8 paint_cursor_x;
static u8 paint_cursor_y;
static u8 paint_color;
static u8 paint_canvas[20 * 14];
static u8 mouse_buttons;
static u8 mouse_prev_buttons;
static u8 mouse_packet_index;
static u8 mouse_packet[3];
static u8 mouse_enabled;
static u8 paint_mouse_drawing;
static u8 network_connected;
static u8 network_ssid;
static u8 desktop_theme;
static u8 start_style;
static char browser_url[64];
static u8 browser_url_length;
static u8 browser_connected;
static u8 first_run;
static char calculator_input[32];
static u8 calculator_input_length;
static char calculator_output[32];
static u8 calculator_error;
static u8 setup_step;
static u8 passcode_input[4];
static u8 passcode_pos;
static s32 window_x;
static s32 window_y;
static u8 window_dragging;
static s32 drag_offset_x;
static s32 drag_offset_y;

static const u8 paint_colors[] = {
    COLOR_BLUE, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
    COLOR_ORANGE, COLOR_CYAN, COLOR_WHITE, COLOR_PANEL_DARK
};

static const char *const network_names[] = {
    "COPPERNET",
    "HOME",
    "GUEST",
};

static const u8 desktop_colors[] = {
    COLOR_DESKTOP,
    COLOR_PANEL_FILL,
    COLOR_CYAN,
};

static const char *const desktop_theme_names[] = {
    "CLASSIC",
    "SILVER",
    "AQUA",
};

static const char *const start_style_names[] = {
    "CLASSIC",
    "MODERN",
};

static const Rect icon_frames[] = {
    {20, 36, 60, 60},
    {116, 36, 60, 60},
    {20, 148, 60, 60},
    {116, 148, 60, 60},
    {20, 260, 60, 60},
    {116, 260, 60, 60},
    {20, 372, 60, 60},
};

static const Rect icon_targets[] = {
    {20, 36, 60, 84},
    {116, 36, 60, 84},
    {20, 148, 60, 84},
    {116, 148, 60, 84},
    {20, 260, 60, 84},
    {116, 260, 60, 84},
    {20, 372, 60, 84},
};

static const s32 icon_pointer_x[] = { 38, 134, 38, 134, 38, 134, 38 };
static const s32 icon_pointer_y[] = { 52, 52, 164, 164, 276, 276, 388 };

static const char glyph_chars[] =
    " .:-/()0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const u8 glyph_data[][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    {0x02,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00},
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00},
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},
    {0x7E,0x66,0x0C,0x18,0x18,0x18,0x18,0x00},
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
    {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00},
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00},
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00},
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00},
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00},
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00},
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
    {0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00},
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00},
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
    {0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x00},
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
};

static inline void outb(u16 port, u8 value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

... (truncated for brevity, the file is large)
</thinking>

**Blinking fix implemented: dirty_flags for pointer-only updates on mouse move (clear old pos + draw new), full on changes. bool fixed. Build issues: syntax in render (missing }); assets nasm ok now (raw exist).**

Use create_file for clean kernel.c, then test.

From read_file, error at render line 1740 expected ; } → missing } for else branch.

From diff, new_str cut off else { clear_screen( 

Yes, incomplete.

Fix render syntax, then assets asm comment out new icons temp (add later with cStore).

First fix kernel.c syntax.

Unique str "clear_screen(desktop_colors[desktop_theme]);"

Then add } after the draw code.

But since large, create full fixed.

Also unused kernel_main warning → remove static or call.

Error "expected declaration or statement at end of input" → unbalanced braces.

From content, in render: if (dirty == DIRTY_POINTER) { fill_rect } else { clear_screen no closing } before draw_text.

Yes.

To fix: edit to add } before draw_text.

Next: Step 2 START replace.

In kernel.c draw_taskbar: extern u8 redsnake_icon[]; draw_raw_sprite(10, taskbar_y+7,48,24,redsnake_icon); instead "START"

Adjust size.

Then Step 3 cStore app.

</thinking>

<xai:function_call name="edit_file">
<parameter name="path">kernel/kernel.c
