#ifndef COPPEROS_BOOTINFO_H
#define COPPEROS_BOOTINFO_H

#include "types.h"

typedef struct BootInfo {
    u32 framebuffer;
    u16 pitch;
    u16 width;
    u16 height;
    u8 bpp;
    u8 mode;
} BootInfo;

#define BOOT_INFO_ADDR ((BootInfo *)0x8000)

#endif
