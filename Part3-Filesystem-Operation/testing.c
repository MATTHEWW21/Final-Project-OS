#include <stdio.h>
#include <string.h>
#include "filesystem_ops.h"

int fs_create(const char *device, fs_type_t type, const char *label) {
    char cmd[512];

    const char *tool = (type == FS_TYPE_EXT4) ? "mkfs.ext4" :
                       (type == FS_TYPE_XFS)  ? "mkfs.xfs"  :
                       (type == FS_TYPE_BTRFS)? "mkfs.btrfs": NULL;

    if (!tool) {
        fprintf(stderr, "FS not support\n");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "sudo %s -L %s %s", tool, label, device);
    return system(cmd);
}
