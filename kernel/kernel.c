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
extern const u8 redsnake_icon[];
extern const u8 cstore_icon[];
extern const u8 txteditor_icon[];

static const char *const version_name = "1.0 Copperhead";
static const char *const build_target = "I386 BIOS UNIX BASE";
static const char *const build_time = __TIME__;

#define BACKBUFFER_MAX_WIDTH 1280
#define BACKBUFFER_MAX_HEIGHT 720

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
    APP_CSTORE = 9,
    APP_TXT_EDITOR = 10,
    APP_LOGIN = 11,
    APP_BOOTLOADER = 12,
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
    TARGET_ICON_CSTORE = 23,
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
    TARGET_ICON_TXT_EDITOR = 20,
    TARGET_MENU_TXT_EDITOR = 21,
    TARGET_MENU_BOOTLOADER = 22,
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
    KEY_8 = 0x09,
    KEY_9 = 0x0A,
    KEY_0 = 0x0B,
    KEY_MINUS = 0x0C,
    KEY_EQUALS = 0x0D,
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
static volatile u8 *g_draw_fb;
static u16 g_width;
static u16 g_height;
static u16 g_pitch;
static u16 g_draw_pitch;
static u8 use_backbuffer;
static u8 clean_frame_ready;
static u8 frame_phase;
static u8 backbuffer[BACKBUFFER_MAX_WIDTH * BACKBUFFER_MAX_HEIGHT];

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
static const char *browser_content;
static u32 browser_content_len;
static u32 browser_display_offset;
static u8 first_run;
static char calculator_input[32];
static u8 calculator_input_length;
static char calculator_output[32];
static u8 calculator_error;
static u8 setup_step;
static u8 setup_passcode[4];
static u8 login_passcode[4];
static u8 passcode_pos;
static u8 passcode_error;
static char user_name[16];
static u8 user_name_length;
static u8 install_choice;
static u8 selected_disk;
static u8 disk_scan_progress;
static u8 login_error;
static char txt_buffer[256];
static u8 txt_length;
static u32 txt_autosave_counter;
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
    {116, 372, 60, 60},
};

static const Rect icon_targets[] = {
    {20, 36, 60, 84},
    {116, 36, 60, 84},
    {20, 148, 60, 84},
    {116, 148, 60, 84},
    {20, 260, 60, 84},
    {116, 260, 60, 84},
    {20, 372, 60, 84},
    {116, 372, 60, 84},
    {20, 484, 60, 84},
};

static const s32 icon_pointer_x[] = { 38, 134, 38, 134, 38, 134, 38, 134, 38 };
static const s32 icon_pointer_y[] = { 52, 52, 164, 164, 276, 276, 388, 388, 500 };

static const char glyph_chars[] =
    " .:-/()+*0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const u8 glyph_data[][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    {0x02,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
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

static void serial_init(void) {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

static void serial_write_char(char ch) {
    while ((inb(0x3F8 + 5) & 0x20) == 0) {
    }
    outb(0x3F8, (u8)ch);
}

static void serial_write_string(const char *text) {
    while (*text) {
        serial_write_char(*text++);
    }
}

static void set_palette(void) {
    u32 r;
    u32 g;
    u32 b;

    outb(0x3C8, 0);
    for (r = 0; r < 6; ++r) {
        for (g = 0; g < 6; ++g) {
            for (b = 0; b < 6; ++b) {
                outb(0x3C9, (u8)(r == 5 ? 63 : r * 12));
                outb(0x3C9, (u8)(g == 5 ? 63 : g * 12));
                outb(0x3C9, (u8)(b == 5 ? 63 : b * 12));
            }
        }
    }

    for (r = 0; r < 40; ++r) {
        u8 value = (u8)(r * 2);
        if (value > 63) {
            value = 63;
        }
        outb(0x3C9, value);
        outb(0x3C9, value);
        outb(0x3C9, value);
    }
}

static void delay_loop(u32 count) {
    volatile u32 i;
    for (i = 0; i < count; ++i) {
    }
}

static void ps2_wait_input(void) {
    while (inb(0x64) & 0x02) {
    }
}

static u8 ps2_read_data(void) {
    while (1) {
        u8 status = inb(0x64);
        if ((status & 0x01) == 0) {
            continue;
        }
        u8 value = inb(0x60);
        if (status & 0x20) {
            return value;
        }
    }
}

static u8 ps2_read_any(void) {
    while ((inb(0x64) & 0x01) == 0) {
    }
    return inb(0x60);
}

static void ps2_write_command(u8 value) {
    ps2_wait_input();
    outb(0x64, value);
}

static void ps2_write_data(u8 value) {
    ps2_wait_input();
    outb(0x60, value);
}

static void enable_mouse(void) {
    ps2_write_command(0xA8); /* Enable auxiliary device */
    ps2_write_command(0x20); /* Read controller command byte */
    u8 command = ps2_read_any();
    command |= 0x20; /* Enable auxiliary device */
    ps2_write_command(0x60);
    ps2_write_data(command);

    ps2_write_command(0xD4);
    ps2_write_data(0xF4); /* Enable data reporting */
    (void)ps2_read_data(); /* Ack */
    mouse_enabled = 1;
}

static void reset_mouse_packet(void) {
    mouse_packet_index = 0;
}

static void update_hover_target(void);
static void activate_current_target(void);
static void handle_calculator_key(u8 key);
static u8 passcode_matches(const u8 *lhs, const u8 *rhs);
static void render(void);

static s32 window_width(void) {
    s32 width = (s32)g_width - 236;
    if (width > 920) {
        width = 920;
    }
    if (width < 404) {
        width = (s32)g_width - 24;
    }
    return width;
}

static s32 window_height(void) {
    s32 height = (s32)g_height - 104;
    if (height > 560) {
        height = 560;
    }
    if (height < 300) {
        height = (s32)g_height - 64;
    }
    return height;
}

static s32 window_content_w(void) {
    return window_width() - 48;
}

static s32 window_content_h(void) {
    return window_height() - 76;
}

static s32 centered_x(s32 w) {
    return ((s32)g_width - w) / 2;
}

static s32 centered_y(s32 h) {
    return ((s32)g_height - h) / 2;
}

static void clamp_window(void) {
    s32 width = window_width();
    s32 height = window_height();

    if (window_x < 0) {
        window_x = 0;
    }
    if (window_y < 0) {
        window_y = 0;
    }
    if (window_x > (s32)g_width - width) {
        window_x = (s32)g_width - width;
    }
    if (window_y > (s32)g_height - height) {
        window_y = (s32)g_height - height;
    }
}

static void handle_mouse_up(void) {
    window_dragging = 0;
    paint_mouse_drawing = 0;

    if (current_screen == APP_DESKTOP) {
        if ((hover_target == TARGET_ICON_FILE || hover_target == TARGET_ICON_WEB ||
             hover_target == TARGET_ICON_ACTIVITY || hover_target == TARGET_ICON_PAINT ||
             hover_target == TARGET_ICON_SETTINGS || hover_target == TARGET_ICON_UPDATES ||
             hover_target == TARGET_ICON_CALCULATOR || hover_target == TARGET_ICON_TXT_EDITOR ||
             hover_target == TARGET_ICON_CSTORE)) {
            activate_current_target();
        }
    }
}

static void handle_mouse_down(void) {
    update_hover_target();
    if (current_screen != APP_DESKTOP) {
        s32 width = window_width();
        if (hover_target == TARGET_WINDOW_CLOSE) {
            current_screen = APP_DESKTOP;
            start_open = 0;
            return;
        }
        if (pointer_x >= window_x + 8 && pointer_x <= window_x + width - 16 &&
            pointer_y >= window_y + 8 && pointer_y <= window_y + 30) {
            window_dragging = 1;
            drag_offset_x = pointer_x - window_x;
            drag_offset_y = pointer_y - window_y;
            return;
        }
    }

    if (current_screen == APP_PAINT) {
        s32 paint_x = window_x + 34;
        s32 paint_y = window_y + 54;
        if (pointer_x >= paint_x && pointer_x < paint_x + 20 * 12 &&
            pointer_y >= paint_y && pointer_y < paint_y + 14 * 12) {
            paint_mouse_drawing = 1;
            u8 col = (u8)((pointer_x - paint_x) / 12);
            u8 row = (u8)((pointer_y - paint_y) / 12);
            paint_canvas[(u32)row * 20u + col] = paint_colors[paint_color];
            return;
        }
    }

    if (current_screen == APP_DESKTOP) {
        if (hover_target == TARGET_START) {
            start_open = (u8)!start_open;
            return;
        }
        if (hover_target == TARGET_ICON_FILE || hover_target == TARGET_ICON_WEB ||
            hover_target == TARGET_ICON_ACTIVITY || hover_target == TARGET_ICON_PAINT ||
            hover_target == TARGET_ICON_SETTINGS || hover_target == TARGET_ICON_UPDATES ||
            hover_target == TARGET_ICON_CALCULATOR || hover_target == TARGET_ICON_TXT_EDITOR ||
            hover_target == TARGET_ICON_CSTORE) {
            return;
        }
        if (start_open) {
            if (hover_target == TARGET_MENU_FILE || hover_target == TARGET_MENU_WEB ||
                hover_target == TARGET_MENU_ACTIVITY || hover_target == TARGET_MENU_PAINT ||
                hover_target == TARGET_MENU_SETTINGS || hover_target == TARGET_MENU_UPDATES ||
                hover_target == TARGET_MENU_CALCULATOR || hover_target == TARGET_MENU_TXT_EDITOR ||
                hover_target == TARGET_MENU_BOOTLOADER || hover_target == TARGET_MENU_SHUTDOWN ||
                hover_target == TARGET_MENU_RESTART || hover_target == TARGET_MENU_SUSPEND) {
                activate_current_target();
                return;
            }
            start_open = 0;
        }
    }
}

static u8 poll_key(void) {
    u8 status = inb(0x64);
    if ((status & 0x01) == 0) {
        return 0;
    }
    if ((status & 0x20) != 0) {
        return 0;
    }
    return inb(0x60);
}

static u8 poll_mouse(void) {
    u8 moved = 0;
    while (1) {
        u8 status = inb(0x64);
        if ((status & 0x01) == 0 || (status & 0x20) == 0) {
            break;
        }

        u8 data = inb(0x60);
        if (mouse_packet_index == 0 && (data & 0x08) == 0) {
            mouse_packet_index = 0;
            continue;
        }

        mouse_packet[mouse_packet_index++] = data;
        if (mouse_packet_index < 3) {
            continue;
        }

        mouse_packet_index = 0;
        if (!(mouse_packet[0] & 0x08)) {
            continue;
        }

        prev_pointer_x = pointer_x;
        prev_pointer_y = pointer_y;
        signed char dx = (signed char)mouse_packet[1];
        signed char dy = (signed char)mouse_packet[2];
        dirty_flags |= DIRTY_POINTER;
        pointer_x += dx;
        pointer_y -= dy;
        if (pointer_x < 0) {
            pointer_x = 0;
        }
        if (pointer_y < 0) {
            pointer_y = 0;
        }
        if (pointer_x > (s32)g_width - 20) {
            pointer_x = (s32)g_width - 20;
        }
        if (pointer_y > (s32)g_height - 20) {
            pointer_y = (s32)g_height - 20;
        }

        u8 buttons = mouse_packet[0] & 0x07;
        if ((buttons & 1) && !(mouse_buttons & 1)) {
            handle_mouse_down();
        }
        if (!(buttons & 1) && (mouse_buttons & 1)) {
            handle_mouse_up();
            dirty_flags |= DIRTY_APP;
        }
        if (window_dragging) {
            window_x = pointer_x - drag_offset_x;
            window_y = pointer_y - drag_offset_y;
            clamp_window();
            dirty_flags |= DIRTY_APP;
        }
        if (paint_mouse_drawing) {
            s32 paint_x = window_x + 34;
            s32 paint_y = window_y + 54;
            if (pointer_x >= paint_x && pointer_x < paint_x + 20 * 12 &&
                pointer_y >= paint_y && pointer_y < paint_y + 14 * 12) {
                u8 col = (u8)((pointer_x - paint_x) / 12);
                u8 row = (u8)((pointer_y - paint_y) / 12);
                paint_canvas[(u32)row * 20u + col] = paint_colors[paint_color];
                dirty_flags |= DIRTY_APP;
            }
        }
        mouse_buttons = buttons;
        moved = 1;
    }
    return moved;
}

static void put_pixel(s32 x, s32 y, u8 color) {
    if (x < 0 || y < 0 || x >= (s32)g_width || y >= (s32)g_height) {
        return;
    }
    g_draw_fb[y * g_draw_pitch + x] = color;
}

static void fill_rect(s32 x, s32 y, s32 w, s32 h, u8 color) {
    s32 row;
    s32 col;
    if (w <= 0 || h <= 0) {
        return;
    }
    for (row = 0; row < h; ++row) {
        s32 py = y + row;
        if (py < 0 || py >= (s32)g_height) {
            continue;
        }
        for (col = 0; col < w; ++col) {
            s32 px = x + col;
            if (px < 0 || px >= (s32)g_width) {
                continue;
            }
            g_draw_fb[py * g_draw_pitch + px] = color;
        }
    }
}

static void begin_frame(void) {
    if (g_width <= BACKBUFFER_MAX_WIDTH && g_height <= BACKBUFFER_MAX_HEIGHT) {
        use_backbuffer = 1;
        g_draw_fb = backbuffer;
        g_draw_pitch = BACKBUFFER_MAX_WIDTH;
    } else {
        use_backbuffer = 0;
        g_draw_fb = g_fb;
        g_draw_pitch = g_pitch;
    }
}

static void present_frame(void) {

    u32 y;
    if (!use_backbuffer) {
        return;
    }
    for (y = 0; y < g_height; ++y) {
        u32 x;
        u8 *dst = (u8 *)g_fb + y * g_pitch;
        const u8 *src = backbuffer + y * BACKBUFFER_MAX_WIDTH;
        for (x = 0; x < g_width; x += 8) {
            if (x + 8 <= g_width) {
                dst[x] = src[x];
                dst[x+1] = src[x+1];
                dst[x+2] = src[x+2];
                dst[x+3] = src[x+3];
                dst[x+4] = src[x+4];
                dst[x+5] = src[x+5];
                dst[x+6] = src[x+6];
                dst[x+7] = src[x+7];
            } else {
                for (; x < g_width; ++x) {
                    dst[x] = src[x];
                }
            }
        }
    }
}

// Helper: only POINTER invalidates when mouse moves.
static void set_dirty_pointer_only(void) {
    dirty_flags = DIRTY_POINTER;
}





static void draw_panel(s32 x, s32 y, s32 w, s32 h) {
    fill_rect(x, y, w, h, COLOR_PANEL_FILL);
    fill_rect(x, y, w, 1, COLOR_PANEL_LIGHT);
    fill_rect(x, y, 1, h, COLOR_PANEL_LIGHT);
    fill_rect(x, y + h - 1, w, 1, COLOR_PANEL_DARK);
    fill_rect(x + w - 1, y, 1, h, COLOR_PANEL_DARK);
}

static const u8 *find_glyph(char ch) {
    u32 i;
    for (i = 0; i < (u32)(sizeof(glyph_chars) - 1); ++i) {
        if (glyph_chars[i] == ch) {
            return glyph_data[i];
        }
    }
    return glyph_data[0];
}

static void draw_char(s32 x, s32 y, char ch, u8 color) {
    const u8 *glyph = find_glyph(ch);
    u32 row;
    for (row = 0; row < 8; ++row) {
        u8 bits = glyph[row];
        u32 col;
        for (col = 0; col < 8; ++col) {
            if (bits & 0x80) {
                put_pixel(x + (s32)col, y + (s32)row, color);
            }
            bits <<= 1;
        }
    }
}

static void draw_text(s32 x, s32 y, const char *text, u8 color) {
    s32 origin_x = x;
    while (*text) {
        if (*text == '\n') {
            x = origin_x;
            y += 10;
        } else {
            draw_char(x, y, *text, color);
            x += 7;
        }
        ++text;
    }
}

static char scancode_to_char(u8 scan) {
    if (scan >= 0x02 && scan <= 0x0B) {
        if (scan == 0x0B) {
            return '0';
        }
        return '1' + (char)(scan - 0x02);
    }
    if (scan >= 0x10 && scan <= 0x19) {
        const char *row = "QWERTYUIOP";
        return row[scan - 0x10];
    }
    if (scan >= 0x1E && scan <= 0x26) {
        const char *row = "ASDFGHJKL";
        return row[scan - 0x1E];
    }
    if (scan >= 0x2C && scan <= 0x32) {
        const char *row = "ZXCVBNM";
        return row[scan - 0x2C];
    }
    if (scan == 0x34) {
        return '.';
    }
    if (scan == 0x35) {
        return '/';
    }
    if (scan == 0x0C) {
        return '-';
    }
    if (scan == 0x0D) {
        return '+';
    }
    if (scan == 0x09) {
        return '*';
    }
    if (scan == 0x39) {
        return ' ';
    }
    return 0;
}

static void draw_raw_sprite(s32 x, s32 y, s32 w, s32 h, const u8 *data) {
    s32 row;
    s32 col;
    for (row = 0; row < h; ++row) {
        s32 py = y + row;
        if (py < 0 || py >= (s32)g_height) {
            continue;
        }
        for (col = 0; col < w; ++col) {
            s32 px = x + col;
            if (px < 0 || px >= (s32)g_width) {
                continue;
            }
            u8 value = data[row * w + col];
            if (value != 255) {
                put_pixel(px, py, value);
            }
        }
    }
}

static void clear_screen(u8 color) {
    fill_rect(0, 0, g_width, g_height, color);
}

static void draw_boot_status(u8 stage) {
    s32 y = g_height - 54;
    fill_rect(0, y, g_width, 54, COLOR_BLACK);
    draw_text(18, y + 6, "STARTING COPPEROS", COLOR_TEXT_LIGHT);
    draw_text(250, y + 6, version_name, COLOR_TEXT_LIGHT);
    draw_text(18, y + 18, stage == 0 ? "READING BOOT LOGO" : "PREPARING DRIVERS",
              stage >= 1 ? COLOR_TEXT_LIGHT : COLOR_STATUS_TEXT);
    draw_text(18, y + 28, "LOADING APPS", stage >= 2 ? COLOR_TEXT_LIGHT : COLOR_STATUS_TEXT);
    draw_text(250, y + 18, "BUILDING USER INTERFACE",
              stage >= 3 ? COLOR_TEXT_LIGHT : COLOR_STATUS_TEXT);
    draw_text(250, y + 28, "FINALIZING DESKTOP",
              stage >= 4 ? COLOR_TEXT_LIGHT : COLOR_STATUS_TEXT);
    draw_text(18, y + 40, "TARGET: ", COLOR_STATUS_TEXT);
    draw_text(68, y + 40, build_target, COLOR_STATUS_TEXT);
    draw_text(250, y + 40, "BUILD: ", COLOR_STATUS_TEXT);
    draw_text(298, y + 40, build_time, COLOR_STATUS_TEXT);
}

static void boot_sequence(void) {
    if (g_width >= 640 && g_height >= 480) {
        clear_screen(COLOR_BLACK);
        draw_raw_sprite(centered_x(640), centered_y(312) - 48, 640, 312, boot_log_screen);
        draw_boot_status(0);
        delay_loop(28000000);

        clear_screen(COLOR_BLACK);
        draw_raw_sprite(centered_x(640), centered_y(312) - 48, 640, 312, boot_screen);
        draw_boot_status(1);
        delay_loop(22000000);
        draw_boot_status(2);
        delay_loop(22000000);
        draw_boot_status(3);
        delay_loop(22000000);
        draw_boot_status(4);
        delay_loop(18000000);
        return;
    }

    clear_screen(COLOR_BLACK);
    draw_text(16, 16, "COPPEROS", COLOR_TEXT_LIGHT);
    draw_text(16, 34, "STARTING", COLOR_STATUS_TEXT);
    delay_loop(12000000);
}

static void update_hover_target(void) {
    hover_target = TARGET_NONE;
    if (current_screen != APP_DESKTOP) {
        s32 width = window_width();
        s32 close_x = window_x + width - 34;
        s32 close_y = window_y + 8;
        if (pointer_x >= close_x && pointer_x <= close_x + 18 &&
            pointer_y >= close_y && pointer_y <= close_y + 14) {
            hover_target = TARGET_WINDOW_CLOSE;
        }
        return;
    }

    if (pointer_x >= 16 && pointer_x <= 82 && pointer_y >= g_height - 30 && pointer_y <= g_height - 6) {
        hover_target = TARGET_START;
        return;
    }

    if (pointer_x >= icon_targets[0].x && pointer_x <= icon_targets[0].x + icon_targets[0].w &&
        pointer_y >= icon_targets[0].y && pointer_y <= icon_targets[0].y + icon_targets[0].h) {
        hover_target = TARGET_ICON_FILE;
        return;
    }
    if (pointer_x >= icon_targets[1].x && pointer_x <= icon_targets[1].x + icon_targets[1].w &&
        pointer_y >= icon_targets[1].y && pointer_y <= icon_targets[1].y + icon_targets[1].h) {
        hover_target = TARGET_ICON_WEB;
        return;
    }
    if (pointer_x >= icon_targets[2].x && pointer_x <= icon_targets[2].x + icon_targets[2].w &&
        pointer_y >= icon_targets[2].y && pointer_y <= icon_targets[2].y + icon_targets[2].h) {
        hover_target = TARGET_ICON_ACTIVITY;
        return;
    }
    if (pointer_x >= icon_targets[3].x && pointer_x <= icon_targets[3].x + icon_targets[3].w &&
        pointer_y >= icon_targets[3].y && pointer_y <= icon_targets[3].y + icon_targets[3].h) {
        hover_target = TARGET_ICON_PAINT;
        return;
    }
    if (pointer_x >= icon_targets[4].x && pointer_x <= icon_targets[4].x + icon_targets[4].w &&
        pointer_y >= icon_targets[4].y && pointer_y <= icon_targets[4].y + icon_targets[4].h) {
        hover_target = TARGET_ICON_SETTINGS;
        return;
    }
    if (pointer_x >= icon_targets[5].x && pointer_x <= icon_targets[5].x + icon_targets[5].w &&
        pointer_y >= icon_targets[5].y && pointer_y <= icon_targets[5].y + icon_targets[5].h) {
        hover_target = TARGET_ICON_UPDATES;
        return;
    }
    if (pointer_x >= icon_targets[6].x && pointer_x <= icon_targets[6].x + icon_targets[6].w &&
        pointer_y >= icon_targets[6].y && pointer_y <= icon_targets[6].y + icon_targets[6].h) {
        hover_target = TARGET_ICON_CALCULATOR;
        return;
    }
    if (pointer_x >= icon_targets[7].x && pointer_x <= icon_targets[7].x + icon_targets[7].w &&
        pointer_y >= icon_targets[7].y && pointer_y <= icon_targets[7].y + icon_targets[7].h) {
        hover_target = TARGET_ICON_TXT_EDITOR;
        return;
    }
    if (pointer_x >= icon_targets[8].x && pointer_x <= icon_targets[8].x + icon_targets[8].w &&
        pointer_y >= icon_targets[8].y && pointer_y <= icon_targets[8].y + icon_targets[8].h) {
        hover_target = TARGET_ICON_CSTORE;
        return;
    }

    if (start_open) {
        s32 menu_top = g_height - 278;
        if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 14 && pointer_y < menu_top + 34) {
            hover_target = TARGET_MENU_FILE;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 34 && pointer_y < menu_top + 54) {
            hover_target = TARGET_MENU_WEB;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 54 && pointer_y < menu_top + 74) {
            hover_target = TARGET_MENU_ACTIVITY;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 74 && pointer_y < menu_top + 94) {
            hover_target = TARGET_MENU_PAINT;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 94 && pointer_y < menu_top + 114) {
            hover_target = TARGET_MENU_SETTINGS;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 114 && pointer_y < menu_top + 134) {
            hover_target = TARGET_MENU_UPDATES;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 134 && pointer_y < menu_top + 154) {
            hover_target = TARGET_MENU_CALCULATOR;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 154 && pointer_y < menu_top + 174) {
            hover_target = TARGET_MENU_TXT_EDITOR;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 174 && pointer_y < menu_top + 194) {
            hover_target = TARGET_MENU_BOOTLOADER;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 194 && pointer_y < menu_top + 214) {
            hover_target = TARGET_MENU_SHUTDOWN;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 214 && pointer_y < menu_top + 234) {
            hover_target = TARGET_MENU_RESTART;
        } else if (pointer_x >= 54 && pointer_x <= 230 && pointer_y >= menu_top + 234 && pointer_y < menu_top + 254) {
            hover_target = TARGET_MENU_SUSPEND;
        }
    }
}

static void open_app(u8 app_id) {
    current_screen = app_id;
    start_open = 0;
}

static void shutdown_system(void);
static void restart_system(void);
static void suspend_system(void);

static void activate_current_target(void) {
    update_hover_target();
    switch (hover_target) {
        case TARGET_ICON_FILE:
        case TARGET_MENU_FILE:
            open_app(APP_FILE);
            break;
        case TARGET_ICON_WEB:
        case TARGET_MENU_WEB:
            open_app(APP_WEB);
            break;
        case TARGET_ICON_ACTIVITY:
        case TARGET_MENU_ACTIVITY:
            open_app(APP_ACTIVITY);
            break;
        case TARGET_ICON_PAINT:
        case TARGET_MENU_PAINT:
            open_app(APP_PAINT);
            break;
        case TARGET_ICON_SETTINGS:
        case TARGET_MENU_SETTINGS:
            open_app(APP_SETTINGS);
            break;
        case TARGET_ICON_UPDATES:
        case TARGET_MENU_UPDATES:
            open_app(APP_UPDATES);
            break;
        case TARGET_ICON_CALCULATOR:
        case TARGET_MENU_CALCULATOR:
            open_app(APP_CALCULATOR);
            break;
        case TARGET_ICON_TXT_EDITOR:
        case TARGET_MENU_TXT_EDITOR:
            open_app(APP_TXT_EDITOR);
            break;
        case TARGET_ICON_CSTORE:
            open_app(APP_CSTORE);
            break;
        case TARGET_MENU_BOOTLOADER:
            open_app(APP_BOOTLOADER);
            break;
        case TARGET_MENU_SHUTDOWN:
            shutdown_system();
            break;
        case TARGET_MENU_RESTART:
            restart_system();
            break;
        case TARGET_MENU_SUSPEND:
            suspend_system();
            break;
        case TARGET_START:
            start_open = (u8)!start_open;
            break;
        default:
            break;
    }
}

static void shutdown_system(void) {
    clear_screen(COLOR_BLACK);
    draw_text(18, 18, "SHUTTING DOWN...", COLOR_TEXT_LIGHT);
    for (;;) {
        __asm__ __volatile__("cli");
        __asm__ __volatile__("hlt");
    }
}

static void restart_system(void) {
    outb(0x64, 0xFE);
    for (;;) {
        __asm__ __volatile__("cli");
        __asm__ __volatile__("hlt");
    }
}

static void suspend_system(void) {
    clear_screen(COLOR_BLACK);
    draw_text(18, 18, "SYSTEM SUSPENDED - PRESS ANY KEY", COLOR_TEXT_LIGHT);
    for (;;) {
        u8 key = poll_key();
        if (key != 0) {
            break;
        }
        __asm__ __volatile__("hlt");
    }
    current_screen = APP_DESKTOP;
    start_open = 0;
    render();
}

static void handle_paint_key(u8 key) {
    u32 index;
    if (key == KEY_ESC) {
        current_screen = APP_DESKTOP;
        return;
    }
    if (key == KEY_TAB) {
        paint_color = (u8)((paint_color + 1) & 7);
        return;
    }
    if (key == KEY_C) {
        u32 i;
        for (i = 0; i < (u32)(20 * 14); ++i) {
            paint_canvas[i] = 0;
        }
        return;
    }
    if (key == KEY_W && paint_cursor_y > 0) {
        --paint_cursor_y;
        return;
    }
    if (key == KEY_S && paint_cursor_y < 13) {
        ++paint_cursor_y;
        return;
    }
    if (key == KEY_A && paint_cursor_x > 0) {
        --paint_cursor_x;
        return;
    }
    if (key == KEY_D && paint_cursor_x < 19) {
        ++paint_cursor_x;
        return;
    }
    if (key == KEY_SPACE) {
        index = (u32)paint_cursor_y * 20u + paint_cursor_x;
        paint_canvas[index] = paint_colors[paint_color];
    }
}

static void attempt_browser_connect(void) {
    static const char demo_example[] =
        "Example Domain\n\n"
        "This domain is for use in examples and documentation.\n"
        "More information...";

    static const char demo_google[] =
        "Google Search\n\n"
        "Search the web with Google\n"
        "The most popular search engine";

    static const char demo_github[] =
        "GitHub\n\n"
        "Build software better, together\n"
        "GitHub is where the world builds software.";

    static const char demo_404[] =
        "404 Not Found\n\n"
        "The page you requested could not be found.\n\n"
        "Try one of these:\n"
        "- http://example.com\n"
        "- http://google.com\n"
        "- http://github.com";

    browser_content = demo_404;
    browser_content_len = 0;

    if (browser_url_length > 0) {
        if ((browser_url[0] == 'h' || browser_url[0] == 'H') &&
            (browser_url[7] == 'e' || browser_url[7] == 'E')) {
            if ((browser_url[10] == 'e' && browser_url[11] == 'x') ||
                (browser_url[10] == 'E' && browser_url[11] == 'x')) {
                browser_content = demo_example;
            } else if ((browser_url[10] == 'g' && browser_url[11] == 'o') ||
                       (browser_url[10] == 'G' && browser_url[11] == 'o')) {
                browser_content = demo_google;
            } else if ((browser_url[10] == 'g' && browser_url[11] == 'i') ||
                       (browser_url[10] == 'G' && browser_url[11] == 'i')) {
                browser_content = demo_github;
            }
        }
    }

    u32 i = 0;
    while (browser_content && browser_content[i]) {
        ++i;
    }
    browser_content_len = i;
    browser_display_offset = 0;
    browser_connected = 1;
}

static void handle_web_key(u8 key) {
    if (key == KEY_ESC) {
        current_screen = APP_DESKTOP;
        start_open = 0;
        return;
    }
    if (key == KEY_ENTER) {
        attempt_browser_connect();
        return;
    }
    if (key == KEY_BACKSPACE) {
        if (browser_url_length > 0) {
            browser_url[--browser_url_length] = 0;
        }
        return;
    }
    char ch = scancode_to_char(key);
    if (ch && browser_url_length < (u8)(sizeof(browser_url) - 1)) {
        browser_url[browser_url_length++] = ch;
        browser_url[browser_url_length] = 0;
    }
}

static void handle_settings_key(u8 key) {
    if (key == KEY_ESC) {
        current_screen = APP_DESKTOP;
        start_open = 0;
        return;
    }
    if (key == KEY_N) {
        network_ssid = (u8)((network_ssid + 1) % (sizeof(network_names) / sizeof(network_names[0])));
        network_connected = 0;
        return;
    }
    if (key == KEY_B) {
        desktop_theme = (u8)((desktop_theme + 1) % (sizeof(desktop_colors) / sizeof(desktop_colors[0])));
        return;
    }
    if (key == KEY_M) {
        start_style = (u8)((start_style + 1) % (sizeof(start_style_names) / sizeof(start_style_names[0])));
        return;
    }
    if (key == KEY_ENTER) {
        browser_connected = 0;
        return;
    }
    if (key == KEY_X) {
        browser_connected = 0;
        current_screen = APP_DESKTOP;
        start_open = 0;
        return;
    }
}

static void show_welcome_animation(void) {
    u32 step;
    for (step = 0; step < 32; ++step) {
        begin_frame();
        clear_screen(desktop_colors[desktop_theme]);
        draw_text(centered_x(210), centered_y(120), "WELCOME TO COPPEROS", COLOR_TEXT_LIGHT);
        draw_text(centered_x(280), centered_y(120) + 30, "APPLYING YOUR UNIX BASED PROFILE", COLOR_TEXT_DARK);
        draw_text(centered_x(190), centered_y(120) + 60, "NO BLINK FRAMEBUFFER UI", COLOR_TEXT_LIGHT);
        draw_panel(centered_x(322), centered_y(120) + 100, 322, 20);
        fill_rect(centered_x(322) + 2, centered_y(120) + 102, (s32)step * 10, 16, COLOR_BLUE);
        draw_text(centered_x(70), centered_y(120) + 128, "LOADING...", COLOR_TEXT_DARK);
        present_frame();
        delay_loop(900000);
    }
    begin_frame();
    clear_screen(desktop_colors[desktop_theme]);
    draw_text(centered_x(105), centered_y(40), "SETUP COMPLETE", COLOR_TEXT_LIGHT);
    draw_text(centered_x(145), centered_y(40) + 20, "OPENING LOGIN SCREEN", COLOR_TEXT_DARK);
    present_frame();
    delay_loop(5000000);
}

static void handle_login_key(u8 key) {
    char ch = scancode_to_char(key);
    if (key == KEY_BACKSPACE && passcode_pos > 0) {
        login_passcode[--passcode_pos] = 0;
        login_error = 0;
        return;
    }
    if (key == KEY_ENTER) {
        if (passcode_pos == 4 && passcode_matches(login_passcode, setup_passcode)) {
            u32 i;
            for (i = 0; i < 4; ++i) {
                login_passcode[i] = 0;
            }
            passcode_pos = 0;
            login_error = 0;
            current_screen = APP_DESKTOP;
            start_open = 0;
            return;
        }
        passcode_pos = 0;
        login_error = 1;
        {
            u32 i;
            for (i = 0; i < 4; ++i) {
                login_passcode[i] = 0;
            }
        }
        return;
    }
    if (ch >= '0' && ch <= '9' && passcode_pos < 4) {
        login_passcode[passcode_pos++] = ch;
        login_error = 0;
    }
}

static void handle_txt_editor_key(u8 key) {
    char ch;
    if (key == KEY_ESC) {
        current_screen = APP_DESKTOP;
        start_open = 0;
        return;
    }
    if (key == KEY_BACKSPACE && txt_length > 0) {
        txt_buffer[--txt_length] = 0;
        ++txt_autosave_counter;
        return;
    }
    if (key == KEY_ENTER && txt_length < (u8)(sizeof(txt_buffer) - 1)) {
        txt_buffer[txt_length++] = '\n';
        txt_buffer[txt_length] = 0;
        ++txt_autosave_counter;
        return;
    }
    ch = scancode_to_char(key);
    if (ch && txt_length < (u8)(sizeof(txt_buffer) - 1)) {
        txt_buffer[txt_length++] = ch;
        txt_buffer[txt_length] = 0;
        ++txt_autosave_counter;
    }
}

static void handle_bootloader_key(u8 key) {
    if (key == KEY_1 || key == KEY_ENTER) {
        current_screen = APP_DESKTOP;
        start_open = 0;
        return;
    }
    if (key == KEY_2) {
        current_screen = APP_SETUP;
        setup_step = 0;
        passcode_pos = 0;
        return;
    }
    if (key == KEY_ESC) {
        current_screen = APP_DESKTOP;
        start_open = 0;
    }
}

static u8 passcode_matches(const u8 *lhs, const u8 *rhs) {
    u8 i;
    for (i = 0; i < 4; ++i) {
        if (lhs[i] != rhs[i]) {
            return 0;
        }
    }
    return 1;
}

static void draw_setup_screen(void) {
    s32 panel_w = 620;
    s32 panel_h = 360;
    s32 x = centered_x(panel_w);
    s32 y = centered_y(panel_h);
    clear_screen(COLOR_BLACK);
    draw_panel(x, y, panel_w, panel_h);
    draw_text(x + 58, y + 32, "COPPEROS SETUP", COLOR_TEXT_LIGHT);
    draw_text(x + 58, y + 52, "UNIX BASED INSTALLER", COLOR_TEXT_DARK);

    if (setup_step == 0) {
        draw_text(x + 58, y + 96, "STEP 1: USERNAME", COLOR_TEXT_DARK);
        draw_panel(x + 58, y + 118, 260, 26);
        fill_rect(x + 60, y + 120, 256, 22, COLOR_WHITE);
        draw_text(x + 66, y + 127, user_name_length > 0 ? user_name : "TYPE YOUR NAME", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 170, "A-Z  BACKSPACE  ENTER: NEXT", COLOR_TEXT_DARK);
    } else if (setup_step == 1) {
        draw_text(x + 58, y + 96, "STEP 2: CREATE PASSCODE", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 120, "PASSCODE: ", COLOR_TEXT_DARK);
        {
          u32 j;
          for (j = 0; j < 4; ++j) {
            draw_char(x + 128 + (s32)j * 12, y + 120, setup_passcode[j] ? '.' : '-', COLOR_TEXT_LIGHT);
          }
        }
        draw_text(x + 58, y + 156, "0-9 DIGITS  BACKSPACE  ENTER: SAVE", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 176,
                  passcode_error ? "PASSCODE MUST BE 4 DIGITS" : "THIS PASSCODE LOCKS YOUR DESKTOP",
                  passcode_error ? COLOR_RED : COLOR_STATUS_TEXT);
    } else if (setup_step == 2) {
        draw_text(x + 58, y + 96, "STEP 3: INSTALL MODE", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 120, install_choice == 0 ? "TRY COPPEROS" : "INSTALL TO HARDDISK", COLOR_TEXT_LIGHT);
        draw_text(x + 58, y + 146, "TAB: CHANGE OPTION", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 166, install_choice == 0 ?
                  "TRY MODE DELETES SESSION CONTENT AFTER REBOOT" :
                  "INSTALL MODE KEEPS YOUR USER PROFILE",
                  COLOR_STATUS_TEXT);
        draw_text(x + 58, y + 196, "ENTER: SCAN DISKS", COLOR_TEXT_DARK);
    } else if (setup_step == 3) {
        draw_text(x + 58, y + 96, "STEP 4: DISK SCAN", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 120, "FOUND DISKS:", COLOR_TEXT_DARK);
        draw_text(x + 78, y + 142, selected_disk == 0 ? "> ATA0 HARDDISK 16GB" : "  ATA0 HARDDISK 16GB", COLOR_TEXT_LIGHT);
        draw_text(x + 78, y + 160, selected_disk == 1 ? "> USB INSTALL MEDIA" : "  USB INSTALL MEDIA", COLOR_TEXT_LIGHT);
        draw_panel(x + 58, y + 194, 260, 18);
        fill_rect(x + 60, y + 196, 20 + (s32)disk_scan_progress * 23, 14, COLOR_GREEN);
        draw_text(x + 58, y + 230, "TAB: SELECT  ENTER: CONTINUE", COLOR_TEXT_DARK);
    } else if (setup_step == 4) {
        draw_text(x + 58, y + 96, "STEP 5: PICK DESKTOP COLOR", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 120, desktop_theme_names[desktop_theme], COLOR_TEXT_LIGHT);
        draw_panel(x + 58, y + 144, 140, 60);
        fill_rect(x + 60, y + 146, 136, 56, desktop_colors[desktop_theme]);
        draw_text(x + 58, y + 224, "TAB: CHANGE  ENTER: NEXT", COLOR_TEXT_DARK);
    } else if (setup_step == 5) {
        draw_text(x + 58, y + 96, "STEP 6: NETWORK PROFILE", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 120, network_names[network_ssid], COLOR_TEXT_LIGHT);
        draw_text(x + 58, y + 140, "STATUS: DRIVER NOT INSTALLED", COLOR_STATUS_TEXT);
        draw_text(x + 58, y + 170, "TAB: CHANGE  ENTER: CONTINUE", COLOR_TEXT_DARK);
    } else {
        draw_text(x + 58, y + 96, "STEP 7: READY", COLOR_TEXT_DARK);
        draw_text(x + 58, y + 120, "USER: ", COLOR_TEXT_DARK);
        draw_text(x + 100, y + 120, user_name, COLOR_TEXT_LIGHT);
        draw_text(x + 58, y + 140, install_choice == 0 ? "MODE: TRY FROM USB" : "MODE: INSTALLED TO HARDDISK", COLOR_TEXT_LIGHT);
        draw_text(x + 58, y + 170, "ENTER: FINISH SETUP", COLOR_TEXT_DARK);
    }
}

static void handle_setup_key(u8 key) {
    if (setup_step == 0) {
        char ch = scancode_to_char(key);
        if (key == KEY_BACKSPACE && user_name_length > 0) {
            user_name[--user_name_length] = 0;
        } else if (key == KEY_ENTER && user_name_length > 0) {
            setup_step = 1;
            passcode_pos = 0;
        } else if (((ch >= 'A' && ch <= 'Z') || ch == ' ') && user_name_length < (u8)(sizeof(user_name) - 1)) {
            user_name[user_name_length++] = ch;
            user_name[user_name_length] = 0;
        }
        return;
    }
    if (setup_step == 1) {
        char ch = scancode_to_char(key);
        if (key == KEY_BACKSPACE && passcode_pos > 0) {
            passcode_pos--;
            setup_passcode[passcode_pos] = 0;
            passcode_error = 0;
        } else if (key == KEY_ENTER) {
            if (passcode_pos == 4) {
                setup_step = 2;
                passcode_error = 0;
                passcode_pos = 0;
            } else {
                passcode_error = 1;
            }
        } else if (ch >= '0' && ch <= '9' && passcode_pos < 4) {
            setup_passcode[passcode_pos++] = ch;
            passcode_error = 0;
        }
        return;
    }
    if (key == KEY_TAB) {
        if (setup_step == 2) {
            install_choice = (u8)!install_choice;
        } else if (setup_step == 3) {
            selected_disk = (u8)!selected_disk;
        } else if (setup_step == 4) {
            desktop_theme = (u8)((desktop_theme + 1) % (sizeof(desktop_colors) / sizeof(desktop_colors[0])));
        } else if (setup_step == 5) {
            network_ssid = (u8)((network_ssid + 1) % (sizeof(network_names) / sizeof(network_names[0])));
        }
        return;
    }
    if (key == KEY_ENTER) {
        if (setup_step == 2) {
            disk_scan_progress = 0;
            setup_step = 3;
            return;
        }
        if (setup_step == 3) {
            if (disk_scan_progress < 10) {
                disk_scan_progress = 10;
            }
            setup_step = 4;
            return;
        }
        if (setup_step == 4) {
            setup_step = 5;
            return;
        }
        if (setup_step == 5) {
            network_connected = 0;
            setup_step = 6;
            return;
        }
        if (setup_step == 6) {
            first_run = 0;
            current_screen = APP_LOGIN;
            start_open = 0;
            show_welcome_animation();
            render();
            return;
        }
    }
    if (key == KEY_ESC && setup_step > 0) {
        setup_step--;
    }
}

static void handle_desktop_key(u8 key) {
    if (key == KEY_I && pointer_y > 8) {
        pointer_y -= 8;
    } else if (key == KEY_K && pointer_y < (s32)g_height - 28) {
        pointer_y += 8;
    } else if (key == KEY_J && pointer_x > 8) {
        pointer_x -= 8;
    } else if (key == KEY_L && pointer_x < (s32)g_width - 28) {
        pointer_x += 8;
    } else if (key == KEY_TAB) {
        selected_icon = (u8)((selected_icon + 1) % 9);
        pointer_x = icon_pointer_x[selected_icon];
        pointer_y = icon_pointer_y[selected_icon];
    } else if (key == KEY_ENTER) {
        activate_current_target();
    } else if (key == KEY_S) {
        start_open = (u8)!start_open;
    } else if (key == KEY_1) {
        open_app(APP_FILE);
    } else if (key == KEY_2) {
        open_app(APP_WEB);
    } else if (key == KEY_3) {
        open_app(APP_ACTIVITY);
    } else if (key == KEY_4) {
        open_app(APP_PAINT);
    } else if (key == KEY_5) {
        open_app(APP_SETTINGS);
    } else if (key == KEY_6) {
        open_app(APP_UPDATES);
    } else if (key == KEY_7) {
        open_app(APP_CALCULATOR);
    } else if (key == KEY_8) {
        open_app(APP_TXT_EDITOR);
    } else if (key == KEY_9) {
        open_app(APP_CSTORE);
    }
}

static void handle_key(u8 key) {
    if (key == 0xE0 || (key & 0x80) != 0) {
        return;
    }

    if (current_screen == APP_SETUP) {
        handle_setup_key(key);
        return;
    }

    if (current_screen == APP_LOGIN) {
        handle_login_key(key);
        return;
    }

    if (current_screen == APP_BOOTLOADER) {
        handle_bootloader_key(key);
        return;
    }

    if (current_screen == APP_PAINT) {
        handle_paint_key(key);
        return;
    }

    if (current_screen == APP_WEB) {
        handle_web_key(key);
        return;
    }

    if (current_screen == APP_SETTINGS) {
        handle_settings_key(key);
        return;
    }

    if (current_screen == APP_CALCULATOR) {
        handle_calculator_key(key);
        return;
    }

    if (current_screen == APP_TXT_EDITOR) {
        handle_txt_editor_key(key);
        return;
    }

    if (key == KEY_ESC) {
        if (current_screen != APP_DESKTOP) {
            current_screen = APP_DESKTOP;
            start_open = 0;
        } else {
            start_open = 0;
        }
        return;
    }

    if (current_screen == APP_DESKTOP) {
        handle_desktop_key(key);
        return;
    }

    if (key == KEY_1) {
        open_app(APP_FILE);
    } else if (key == KEY_2) {
        open_app(APP_WEB);
    } else if (key == KEY_3) {
        open_app(APP_ACTIVITY);
    } else if (key == KEY_4) {
        open_app(APP_PAINT);
    } else if (key == KEY_5) {
        open_app(APP_SETTINGS);
    } else if (key == KEY_6) {
        open_app(APP_UPDATES);
    } else if (key == KEY_7) {
        open_app(APP_CALCULATOR);
    } else if (key == KEY_8) {
        open_app(APP_TXT_EDITOR);
    }
}

static void draw_icon_block(s32 idx, const u8 *icon, const char *label1, const char *label2, u8 selected) {
    const Rect *frame = &icon_frames[idx];
    if (selected) {
        draw_panel(frame->x, frame->y, frame->w, frame->h);
    }
    draw_raw_sprite(frame->x + 6, frame->y + 6, 48, 48, icon);
    draw_text(frame->x + 2, frame->y + 58, label1, COLOR_TEXT_LIGHT);
    draw_text(frame->x + 2, frame->y + 68, label2, COLOR_TEXT_LIGHT);
}

static void draw_taskbar(void) {
    s32 taskbar_y = g_height - 38;
    draw_panel(0, taskbar_y, g_width, 38);
    draw_raw_sprite(10, taskbar_y + 7, 48, 24, redsnake_icon);
    if (start_open) {
        fill_rect(12, taskbar_y + 9, 70, 20,
            start_style == 1 ? COLOR_BLUE : COLOR_START_GREEN);
    }
    draw_text(108, taskbar_y + 14, "LMB CLICK  /  1-8 APPS", COLOR_TEXT_DARK);
}

static void draw_start_menu_item(s32 x, s32 y, const char *text, u8 hot_target) {
    if (hover_target == hot_target) {
        fill_rect(x, y, 166, 18, COLOR_BLUE);
        draw_text(x + 6, y + 5, text, COLOR_TEXT_LIGHT);
    } else {
        draw_text(x + 6, y + 5, text, COLOR_TEXT_DARK);
    }
}

static void draw_start_menu(void) {
    s32 x = 12;
    s32 y = g_height - 278;
    draw_panel(x, y, 236, 278);
    fill_rect(x + 6, y + 6, 28, 266, COLOR_BLUE);
    draw_text(x + 10, y + 14, "COP", COLOR_TEXT_LIGHT);
    draw_text(x + 10, y + 28, "OS", COLOR_TEXT_LIGHT);

    draw_start_menu_item(x + 42, y + 14, "FILE EXPLORER", TARGET_MENU_FILE);
    draw_start_menu_item(x + 42, y + 34, "INTERNET EXPLORER", TARGET_MENU_WEB);
    draw_start_menu_item(x + 42, y + 54, "ACTIVITY MANAGER", TARGET_MENU_ACTIVITY);
    draw_start_menu_item(x + 42, y + 74, "PAINT", TARGET_MENU_PAINT);
    draw_start_menu_item(x + 42, y + 94, "SYSTEM SETTINGS", TARGET_MENU_SETTINGS);
    draw_start_menu_item(x + 42, y + 114, "SYSTEM UPDATES", TARGET_MENU_UPDATES);
    draw_start_menu_item(x + 42, y + 134, "CALCULATOR", TARGET_MENU_CALCULATOR);
    draw_start_menu_item(x + 42, y + 154, "TXT EDITOR", TARGET_MENU_TXT_EDITOR);
    draw_start_menu_item(x + 42, y + 174, "REBOOT TO BOOTLOADER", TARGET_MENU_BOOTLOADER);
    draw_start_menu_item(x + 42, y + 194, "SHUTDOWN", TARGET_MENU_SHUTDOWN);
    draw_start_menu_item(x + 42, y + 214, "RESTART", TARGET_MENU_RESTART);
    draw_start_menu_item(x + 42, y + 234, "SUSPEND", TARGET_MENU_SUSPEND);
}

static void draw_window_frame(const char *title) {
    s32 width = window_width();
    s32 height = window_height();
    draw_panel(window_x, window_y, width, height);
    fill_rect(window_x + 8, window_y + 8, width - 16, 22, COLOR_BLUE);
    fill_rect(window_x + 8, window_y + 34, width - 16, height - 42, COLOR_WINDOW_FILL);
    draw_text(window_x + 16, window_y + 15, title, COLOR_TEXT_LIGHT);
    if (hover_target == TARGET_WINDOW_CLOSE) {
        fill_rect(window_x + width - 33, window_y + 9, 16, 12, COLOR_RED);
    }
    draw_panel(window_x + width - 34, window_y + 8, 18, 14);
    draw_text(window_x + width - 29, window_y + 11, "X", hover_target == TARGET_WINDOW_CLOSE ? COLOR_TEXT_LIGHT : COLOR_TEXT_DARK);
}

static void draw_file_app(void) {
    draw_window_frame("FILE EXPLORER");
    s32 base_x = window_x + 24;
    s32 base_y = window_y + 54;
    s32 content_h = window_content_h();
    s32 right_w = window_content_w() - 174;
    draw_panel(base_x, base_y, 156, content_h);
    draw_panel(base_x + 170, base_y, right_w, content_h);
    draw_text(base_x + 8, base_y + 10, "COPPEROS/\nBOOT/\nKERNEL/\nAPPS/\nDOCS/\nASSETS/\nMAKEFILE", COLOR_TEXT_DARK);
    draw_text(base_x + 180, base_y + 10, "PROJECT VIEW\nREAL BOOT IMAGE\nC KERNEL STRENGTHENED\nUPDATED UI APPS\nNEXT: FILESYSTEM", COLOR_TEXT_DARK);
}

static void draw_web_app(void) {
    draw_window_frame("INTERNET EXPLORER");
    s32 base_x = window_x + 24;
    s32 base_y = window_y + 54;
    s32 content_w = window_content_w();
    s32 content_h = window_content_h();

    fill_rect(base_x, base_y, content_w, 20, COLOR_WHITE);
    draw_text(base_x + 6, base_y + 5,
              browser_url_length > 0 ? browser_url : "HTTP://EXAMPLE.COM",
              COLOR_TEXT_DARK);

    if (browser_connected && browser_content) {
        draw_panel(base_x + 4, base_y + 30, content_w - 8, content_h - 40);
        fill_rect(base_x + 6, base_y + 32, content_w - 12, content_h - 44, COLOR_WHITE);
        draw_text(base_x + 8, base_y + 36, browser_content, COLOR_TEXT_DARK);
    } else {
        draw_text(base_x + 4, base_y + 36, "ENTER A URL AND PRESS ENTER", COLOR_TEXT_DARK);
        draw_text(base_x + 4, base_y + 54,
                  "DEMO SITES:\n"
                  "HTTP://EXAMPLE.COM\n"
                  "HTTP://GOOGLE.COM\n"
                  "HTTP://GITHUB.COM",
                  COLOR_TEXT_DARK);
    }
}

static void draw_activity_app(void) {
    char buffer[16];
    u32 value;
    u32 i;
    draw_window_frame("ACTIVITY MANAGER");
    s32 base_x = window_x + 28;
    s32 base_y = window_y + 54;
    draw_text(base_x, base_y, "FRAME LOOPS:", COLOR_TEXT_DARK);
    value = loop_counter;
    for (i = 0; i < 15; ++i) {
        buffer[i] = 0;
    }
    i = 14;
    buffer[i] = 0;
    if (value == 0) {
        buffer[--i] = '0';
    } else {
        while (value != 0 && i != 0) {
            buffer[--i] = (char)('0' + (value % 10));
            value /= 10;
        }
    }
    draw_text(base_x + 98, base_y, &buffer[i], COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 16, "FOREGROUND APP:", COLOR_TEXT_DARK);
    draw_text(base_x + 116, base_y + 16,
              current_screen == APP_FILE ? "FILE EXPLORER" :
              current_screen == APP_WEB ? "INTERNET EXPLORER" :
              current_screen == APP_ACTIVITY ? "ACTIVITY MANAGER" :
              current_screen == APP_PAINT ? "PAINT" :
              current_screen == APP_SETTINGS ? "SYSTEM SETTINGS" :
              current_screen == APP_UPDATES ? "SYSTEM UPDATES" :
              current_screen == APP_CALCULATOR ? "CALCULATOR" :
              current_screen == APP_TXT_EDITOR ? "TXT EDITOR" :
              current_screen == APP_BOOTLOADER ? "BOOTLOADER" :
              "DESKTOP",
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 44,
              "ARCH: 32 BIT C KERNEL\n"
              "POINTER: KEYBOARD + MOUSE\n"
              "DISPLAY: VESA SCALABLE 8 BPP",
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 84,
              mouse_enabled ? "MOUSE: READY\n" : "MOUSE: NOT READY\n",
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 104,
              "NETWORK: DRIVER MISSING\n",
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 124,
              "SYSTEM DIAGNOSTIC: GOOD\n"
              "THEME: ",
              COLOR_TEXT_DARK);
    draw_text(base_x + 76, base_y + 124,
              desktop_theme_names[desktop_theme],
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 144,
              "VERSION: ",
              COLOR_TEXT_DARK);
    draw_text(base_x + 76, base_y + 144,
              version_name,
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 160,
              "BUILD: ",
              COLOR_TEXT_DARK);
    draw_text(base_x + 60, base_y + 160,
              build_target,
              COLOR_TEXT_DARK);
    draw_text(base_x + 128, base_y + 160,
              build_time,
              COLOR_TEXT_DARK);
}

static void draw_settings_app(void) {
    draw_window_frame("SYSTEM SETTINGS");
    s32 base_x = window_x + 28;
    s32 base_y = window_y + 54;
    draw_text(base_x, base_y, "NETWORK: ", COLOR_TEXT_DARK);
    draw_text(base_x + 74, base_y, network_names[network_ssid], COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 16, "THEME: ", COLOR_TEXT_DARK);
    draw_text(base_x + 56, base_y + 16, desktop_theme_names[desktop_theme], COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 32, "START MENU: ", COLOR_TEXT_DARK);
    draw_text(base_x + 96, base_y + 32, start_style_names[start_style], COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 58,
              "N: Change Wi-Fi   B: Change Theme\n"
              "M: Change Start Menu\n"
              "ENTER: Check Real Network Stack\n"
              "X: Apply and Close\n"
              "ESC: Back to Desktop",
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 122,
              "DIAGNOSTICS:\n"
              "SYSTEM HEALTH: GOOD\n"
              "NIC DRIVER: MISSING\n"
              "TCP/IP STACK: MISSING\n"
              "WIFI PROFILE: ",
              COLOR_TEXT_DARK);
    draw_text(base_x + 104, base_y + 152,
              network_names[network_ssid],
              COLOR_TEXT_DARK);
}

static void set_calculator_output(const char *text) {
    u32 i;
    for (i = 0; i < (u32)(sizeof(calculator_output) - 1) && text[i]; ++i) {
        calculator_output[i] = text[i];
    }
    calculator_output[i] = 0;
}

static void calculate_calculator_result(void) {
    s32 total = 0;
    s32 current = 0;
    char op = '+';
    u32 i;

    calculator_error = 0;
    if (calculator_input_length == 0) {
        set_calculator_output("0");
        return;
    }

    for (i = 0; i < calculator_input_length; ++i) {
        char ch = calculator_input[i];
        if (ch >= '0' && ch <= '9') {
            current = current * 10 + (s32)(ch - '0');
            continue;
        }
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            if (op == '+') {
                total += current;
            } else if (op == '-') {
                total -= current;
            } else if (op == '*') {
                total *= current;
            } else if (op == '/') {
                if (current == 0) {
                    calculator_error = 1;
                    break;
                }
                total /= current;
            }
            current = 0;
            op = ch;
            continue;
        }
        calculator_error = 1;
        break;
    }

    if (!calculator_error) {
        if (op == '+') {
            total += current;
        } else if (op == '-') {
            total -= current;
        } else if (op == '*') {
            total *= current;
        } else if (op == '/') {
            if (current == 0) {
                calculator_error = 1;
            } else {
                total /= current;
            }
        }
    }

    if (calculator_error) {
        set_calculator_output("ERROR");
        return;
    }

    char buffer[16];
    u32 index = sizeof(buffer) - 1;
    buffer[index] = 0;
    if (total == 0) {
        buffer[--index] = '0';
    } else {
        s32 value = total;
        u8 negative = 0;
        if (value < 0) {
            negative = 1;
            value = -value;
        }
        while (value > 0 && index > 0) {
            buffer[--index] = (char)('0' + (value % 10));
            value /= 10;
        }
        if (negative && index > 0) {
            buffer[--index] = '-';
        }
    }
    set_calculator_output(&buffer[index]);
}

static void handle_calculator_key(u8 key) {
    if (key == KEY_ESC) {
        current_screen = APP_DESKTOP;
        start_open = 0;
        return;
    }
    if (key == KEY_C) {
        calculator_input_length = 0;
        calculator_input[0] = 0;
        set_calculator_output("0");
        calculator_error = 0;
        return;
    }
    if (key == KEY_ENTER) {
        calculate_calculator_result();
        return;
    }
    char ch = scancode_to_char(key);
    if (ch && (u32)calculator_input_length + 1 < sizeof(calculator_input)) {
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            if (calculator_input_length == 0) {
                return;
            }
            char last = calculator_input[calculator_input_length - 1];
            if (last == '+' || last == '-' || last == '*' || last == '/') {
                return;
            }
        }
        calculator_input[calculator_input_length++] = ch;
        calculator_input[calculator_input_length] = 0;
    }
}

static void draw_updates_app(void) {
    draw_window_frame("SYSTEM UPDATES");
    s32 base_x = window_x + 28;
    s32 base_y = window_y + 54;
    draw_text(base_x, base_y, "CHECKING LOCAL RELEASE METADATA...", COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 20, "THIS IS THE INSTALLED VERSION.", COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 40, "VERSION: ", COLOR_TEXT_DARK);
    draw_text(base_x + 68, base_y + 40, version_name, COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 76,
              "ONLINE UPDATE CHECK REQUIRES REAL NETWORK STACK\n"
              "ESC: BACK TO DESKTOP",
              COLOR_TEXT_DARK);
}

static void draw_calculator_app(void) {
    draw_window_frame("CALCULATOR");
    s32 base_x = window_x + 28;
    s32 base_y = window_y + 54;
    draw_text(base_x, base_y, "TYPE 0-9 + - * /", COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 16, "ENTER=EVAL  C=CLEAR  ESC=BACK", COLOR_TEXT_DARK);
    draw_panel(base_x, base_y + 42, 228, 32);
    fill_rect(base_x + 2, base_y + 44, 224, 28, COLOR_WHITE);
    draw_text(base_x + 6, base_y + 48,
              calculator_input_length > 0 ? calculator_input : "0",
              COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 84, "RESULT:", COLOR_TEXT_DARK);
    draw_text(base_x + 66, base_y + 84,
              calculator_output,
              COLOR_TEXT_DARK);
}

static void draw_cstore_app(void) {
    draw_window_frame("CSTORE");
    s32 base_x = window_x + 28;
    s32 base_y = window_y + 54;
    draw_text(base_x, base_y, "ERROR", COLOR_TEXT_DARK);
    draw_text(base_x, base_y + 20, "CAN'T FIND CSTORE:APPLICATION/BINARY", COLOR_TEXT_DARK);
    draw_panel(base_x + 80, base_y + 60, 60, 24);
    fill_rect(base_x + 82, base_y + 62, 56, 20, COLOR_TEXT_DARK);
    draw_text(base_x + 93, base_y + 66, "OK", COLOR_TEXT_LIGHT);
}

static void draw_txt_editor_app(void) {
    s32 base_x;
    s32 base_y;
    s32 i;
    s32 x;
    s32 y;
    draw_window_frame("TXT EDITOR");
    base_x = window_x + 28;
    base_y = window_y + 54;
    s32 content_w = window_content_w();
    s32 content_h = window_content_h();
    draw_text(base_x, base_y, "AUTO SAVE: ON", COLOR_TEXT_DARK);
    draw_text(base_x + 116, base_y, "EDITS:", COLOR_TEXT_DARK);
    {
        char buffer[12];
        u32 value = txt_autosave_counter;
        u32 idx = sizeof(buffer) - 1;
        buffer[idx] = 0;
        if (value == 0) {
            buffer[--idx] = '0';
        } else {
            while (value > 0 && idx > 0) {
                buffer[--idx] = (char)('0' + (value % 10));
                value /= 10;
            }
        }
        draw_text(base_x + 168, base_y, &buffer[idx], COLOR_TEXT_DARK);
    }
    draw_panel(base_x, base_y + 24, content_w, content_h - 34);
    fill_rect(base_x + 2, base_y + 26, content_w - 4, content_h - 38, COLOR_WHITE);
    x = base_x + 8;
    y = base_y + 34;
    if (txt_length == 0) {
        draw_text(x, y, "START TYPING...", COLOR_STATUS_TEXT);
    }
    for (i = 0; i < (s32)txt_length; ++i) {
        if (txt_buffer[i] == '\n' || x > base_x + content_w - 26) {
            x = base_x + 8;
            y += 10;
            if (txt_buffer[i] == '\n') {
                continue;
            }
        }
        if (y < base_y + content_h - 26) {
            draw_char(x, y, txt_buffer[i], COLOR_TEXT_DARK);
        }
        x += 7;
    }
    draw_text(base_x, base_y + content_h - 2, "ESC: BACK  ENTER: NEW LINE", COLOR_TEXT_DARK);
}

static void draw_paint_app(void) {
    u32 row;
    u32 col;
    draw_window_frame("PAINT");
    s32 base_x = window_x + 34;
    s32 base_y = window_y + 54;
    draw_text(window_x + 28, window_y + 58, "LMB DRAW  TAB COLOR  C CLEAR", COLOR_TEXT_DARK);
    for (row = 0; row < 14; ++row) {
        for (col = 0; col < 20; ++col) {
            u8 color = paint_canvas[row * 20 + col];
            s32 x = base_x + (s32)col * 12;
            s32 y = base_y + (s32)row * 12;
            if (color == 0) {
                color = COLOR_WHITE;
            }
            fill_rect(x, y, 10, 10, color);
            if (col == paint_cursor_x && row == paint_cursor_y) {
                draw_panel(x - 1, y - 1, 12, 12);
            }
        }
    }
    draw_panel(base_x + 276, base_y, 34, 34);
    fill_rect(base_x + 283, base_y + 7, 20, 20, paint_colors[paint_color]);
}

static void draw_login_screen(void) {
    clear_screen(COLOR_BLACK);
    draw_panel((s32)g_width / 2 - 170, (s32)g_height / 2 - 96, 340, 192);
    draw_text((s32)g_width / 2 - 116, (s32)g_height / 2 - 68, "COPPEROS LOGIN", COLOR_TEXT_LIGHT);
    draw_text((s32)g_width / 2 - 116, (s32)g_height / 2 - 42, "USER:", COLOR_TEXT_DARK);
    draw_text((s32)g_width / 2 - 70, (s32)g_height / 2 - 42, user_name, COLOR_TEXT_LIGHT);
    draw_text((s32)g_width / 2 - 116, (s32)g_height / 2 - 16, "PASSCODE:", COLOR_TEXT_DARK);
    {
        u32 i;
        for (i = 0; i < 4; ++i) {
            draw_char((s32)g_width / 2 - 38 + (s32)i * 14, (s32)g_height / 2 - 16,
                      login_passcode[i] ? '.' : '-', COLOR_TEXT_LIGHT);
        }
    }
    draw_text((s32)g_width / 2 - 116, (s32)g_height / 2 + 22,
              login_error ? "WRONG PASSCODE" : "ENTER YOUR SETUP PASSCODE",
              login_error ? COLOR_RED : COLOR_STATUS_TEXT);
}

static void draw_bootloader_screen(void) {
    clear_screen(COLOR_BLACK);
    draw_text(32, 36, "COPPEROS BOOTLOADER", COLOR_TEXT_LIGHT);
    draw_text(32, 58, "CHOOSE ONE OPERATING SYSTEM", COLOR_STATUS_TEXT);
    draw_panel(32, 94, 380, 96);
    draw_text(54, 116, "> 1  COPPEROS 1.0 COPPERHEAD", COLOR_TEXT_LIGHT);
    draw_text(54, 140, "  2  USB CUSTOM OPERATING SYSTEM INSTALLER", COLOR_TEXT_DARK);
    draw_text(32, 220, "USB: DETECTED AS INSTALL MEDIA", COLOR_TEXT_LIGHT);
    draw_text(32, 242, "ENTER: BOOT SELECTED  ESC: RETURN", COLOR_STATUS_TEXT);
}

static void draw_pointer(void) {
    u8 fill_color = hover_target == TARGET_NONE ? COLOR_WHITE : COLOR_BLUE;
    u8 border_color = COLOR_BLACK;
    s32 x = pointer_x;
    s32 y = pointer_y;
    s32 row;

    for (row = 0; row < 12; ++row) {
        s32 col;
        for (col = 0; col <= row; ++col) {
            put_pixel(x + col, y + row, fill_color);
        }
    }

    fill_rect(x + 2, y + 12, 4, 6, fill_color);

    for (row = 0; row < 12; ++row) {
        put_pixel(x + row, y + row, border_color);
    }
    for (row = 12; row < 18; ++row) {
        put_pixel(x + 2, y + row, border_color);
        put_pixel(x + 5, y + row, border_color);
    }
    put_pixel(x + 3, y + 18, border_color);
    put_pixel(x + 4, y + 18, border_color);
    put_pixel(x + 2, y + 17, border_color);
}

void render(void) {
    begin_frame();
    ++frame_phase;
    update_hover_target();
    if (current_screen == APP_SETUP) {
        draw_setup_screen();
        draw_pointer();
        present_frame();
        clean_frame_ready = 0;
        return;
    }
    if (current_screen == APP_LOGIN) {
        draw_login_screen();
        draw_pointer();
        present_frame();
        clean_frame_ready = 0;
        return;
    }
    if (current_screen == APP_BOOTLOADER) {
        draw_bootloader_screen();
        draw_pointer();
        present_frame();
        clean_frame_ready = 0;
        return;
    }

    clear_screen(desktop_colors[desktop_theme]);

    draw_text(20, 12, "COPPEROS", COLOR_TEXT_LIGHT);
    draw_text(20, 26, "HD FRAMEBUFFER UI + NO BLINK EVENT REDRAW", COLOR_STATUS_TEXT);

    draw_icon_block(0, filemanager_icon, "FILE", "EXPLORER", selected_icon == 0);
    draw_icon_block(1, internetexplorer_icon, "INTERNET", "EXPLORER", selected_icon == 1);
    draw_icon_block(2, activitymanager_icon, "ACTIVITY", "MANAGER", selected_icon == 2);
    draw_icon_block(3, paint_icon, "PAINT", "", selected_icon == 3);
    draw_icon_block(4, systemsettings_icon, "SYSTEM", "SETTINGS", selected_icon == 4);
    draw_icon_block(5, systemupdates_icon, "SYSTEM", "UPDATES", selected_icon == 5);
    draw_icon_block(6, calculator_icon, "CALC", "ULATOR", selected_icon == 6);
    draw_icon_block(7, txteditor_icon, "TXT", "EDITOR", selected_icon == 7);
    draw_icon_block(8, cstore_icon, "COPPER", "STORE", selected_icon == 8);
    draw_taskbar();

    if (current_screen == APP_FILE) {
        draw_file_app();
    } else if (current_screen == APP_WEB) {
        draw_web_app();
    } else if (current_screen == APP_ACTIVITY) {
        draw_activity_app();
    } else if (current_screen == APP_SETTINGS) {
        draw_settings_app();
    } else if (current_screen == APP_UPDATES) {
        draw_updates_app();
    } else if (current_screen == APP_CALCULATOR) {
        draw_calculator_app();
    } else if (current_screen == APP_CSTORE) {
        draw_cstore_app();
    } else if (current_screen == APP_TXT_EDITOR) {
        draw_txt_editor_app();
    } else if (current_screen == APP_PAINT) {
        draw_paint_app();
    } else if (start_open) {
        draw_start_menu();
    }

    draw_pointer();
    present_frame();
    clean_frame_ready = use_backbuffer;
}

void kernel_main(BootInfo *boot) {
    u32 i;
    g_boot = boot;
    g_fb = (volatile u8 *)(u32)boot->framebuffer;
    g_width = boot->width;
    g_height = boot->height;
    g_pitch = boot->pitch;
    g_draw_fb = g_fb;
    g_draw_pitch = g_pitch;
    use_backbuffer = 0;
    clean_frame_ready = 0;
    frame_phase = 0;

    serial_init();
    set_palette();
    boot_sequence();

    mouse_enabled = 0;
    mouse_buttons = 0;
    mouse_prev_buttons = 0;
    paint_mouse_drawing = 0;
    window_dragging = 0;
    drag_offset_x = 0;
    drag_offset_y = 0;
    window_x = centered_x(window_width());
    window_y = centered_y(window_height());
    reset_mouse_packet();
    enable_mouse();

    current_screen = APP_SETUP;
    selected_icon = 0;
    start_open = 0;
    pointer_x = icon_pointer_x[0];
    pointer_y = icon_pointer_y[0];
    prev_pointer_x = pointer_x;
    prev_pointer_y = pointer_y;
    hover_target = TARGET_ICON_FILE;
    dirty_flags = DIRTY_FULL;
    paint_cursor_x = 0;
    paint_cursor_y = 0;
    paint_color = 0;
first_run = 1;
setup_step = 0;
passcode_pos = 0;
passcode_error = 0;
login_error = 0;
user_name_length = 0;
user_name[0] = 0;
install_choice = 1;
selected_disk = 0;
disk_scan_progress = 0;
u32 j;
for (j = 0; j < 4; ++j) {
  setup_passcode[j] = 0;
  login_passcode[j] = 0;
}
network_connected = 0;
    network_ssid = 0;
    desktop_theme = 0;
    start_style = 0;
    browser_url_length = 0;
    browser_url[0] = 0;
    browser_connected = 0;
    browser_content = 0;
    browser_content_len = 0;
    browser_display_offset = 0;
    calculator_input_length = 0;
    calculator_input[0] = 0;
    calculator_output[0] = '0';
    calculator_output[1] = 0;
    calculator_error = 0;
    txt_length = 0;
    txt_buffer[0] = 0;
    txt_autosave_counter = 0;
    for (i = 0; i < (u32)(20 * 14); ++i) {
        paint_canvas[i] = 0;
    }

    render();
    serial_write_string("CopperOS booted\r\n");

for (;;) {
        u8 key = poll_key();
        ++loop_counter;
u8 mouse_moved = poll_mouse();
if (mouse_moved && dirty_flags == DIRTY_NONE) dirty_flags |= DIRTY_POINTER;
u8 do_full_update = ((dirty_flags & DIRTY_APP) != 0);
        if (key != 0) {
            u8 key_causes_full_update = 0;
            if (key == KEY_ESC || key == KEY_ENTER || key == KEY_S) {
                key_causes_full_update = 1;
            }
            if (current_screen == APP_DESKTOP && (key == KEY_TAB || key == KEY_I || key == KEY_J || key == KEY_K || key == KEY_L)) {
                key_causes_full_update = 1;
            }
            if (current_screen == APP_SETTINGS && (key == KEY_N || key == KEY_B || key == KEY_M || key == KEY_X)) {
                key_causes_full_update = 1;
            }

            handle_key(key);

            if (key_causes_full_update) {
                dirty_flags |= DIRTY_FULL;
                do_full_update = true;
            } else {
                dirty_flags |= DIRTY_APP;
                do_full_update = true;
            }
        }
        if (current_screen == APP_SETUP && setup_step == 3 && disk_scan_progress < 10 &&
            (loop_counter & 0x3FFFu) == 0) {
            ++disk_scan_progress;
            dirty_flags |= DIRTY_FULL;
            do_full_update = true;
        }
        if (do_full_update || dirty_flags != DIRTY_NONE) {
            render();
            dirty_flags = DIRTY_NONE;
        }
    }
}
