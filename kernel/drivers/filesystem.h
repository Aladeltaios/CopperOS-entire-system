#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

#define FS_MAX_ITEMS 16
#define FS_ITEM_NAME_LEN 32

typedef struct {
    s32 x;
    s32 y;
    u32 w;
    u32 h;
} FSRect;

typedef struct {
    char name[FS_ITEM_NAME_LEN];
    u8 is_folder;
    u8 is_app;
    u8 app_id;
    u8 parent_id;
    FSRect pos;
} FSItem;

typedef struct {
    u32 magic;
    u32 version;
    u32 item_count;
    FSItem items[FS_MAX_ITEMS];
} FSConfig;

void fs_init(void);
void fs_add_item(const char *name, u8 is_folder, u8 app_id, s32 x, s32 y);
void fs_update_item_pos(u32 idx, s32 x, s32 y);
FSItem *fs_get_item(u32 idx);
u32 fs_item_count(void);

#endif
