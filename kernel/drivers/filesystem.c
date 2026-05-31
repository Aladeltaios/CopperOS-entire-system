#include "../include/types.h"
#include "filesystem.h"

#define FS_CONFIG_SECTOR 100

static FSConfig fs_config = {
    .magic = 0xCOPPEROSu,
    .version = 1,
    .item_count = 0,
};

void fs_init(void) {
    u32 i;
    for (i = 0; i < FS_MAX_ITEMS; ++i) {
        fs_config.items[i].name[0] = 0;
        fs_config.items[i].is_folder = 0;
        fs_config.items[i].is_app = 0;
        fs_config.items[i].app_id = 0;
        fs_config.items[i].parent_id = 0;
    }
    fs_config.item_count = 0;
}

void fs_add_item(const char *name, u8 is_folder, u8 app_id, s32 x, s32 y) {
    u32 idx = fs_config.item_count;
    if (idx >= FS_MAX_ITEMS) return;

    u32 i = 0;
    while (name[i] && i < FS_ITEM_NAME_LEN - 1) {
        fs_config.items[idx].name[i] = name[i];
        ++i;
    }
    fs_config.items[idx].name[i] = 0;

    fs_config.items[idx].is_folder = is_folder;
    fs_config.items[idx].is_app = !is_folder;
    fs_config.items[idx].app_id = app_id;
    fs_config.items[idx].parent_id = 0;
    fs_config.items[idx].pos.x = x;
    fs_config.items[idx].pos.y = y;
    fs_config.items[idx].pos.w = 60;
    fs_config.items[idx].pos.h = 60;

    ++fs_config.item_count;
}

void fs_update_item_pos(u32 idx, s32 x, s32 y) {
    if (idx < FS_MAX_ITEMS) {
        fs_config.items[idx].pos.x = x;
        fs_config.items[idx].pos.y = y;
    }
}

FSItem *fs_get_item(u32 idx) {
    if (idx < FS_MAX_ITEMS) {
        return &fs_config.items[idx];
    }
    return 0;
}

u32 fs_item_count(void) {
    return fs_config.item_count;
}
