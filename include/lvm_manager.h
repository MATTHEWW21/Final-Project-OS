#ifndef LVM_MANAGER_H
#define LVM_MANAGER_H

typedef struct {
    char pv_name[64];
    unsigned long long size_bytes;
    char vg_name[64];
} pv_info_t;

typedef struct {
    char vg_name[64];
    unsigned long long size_bytes;
    unsigned long long free_bytes;
    int pv_count;
    int lv_count;
} vg_info_t;

typedef struct {
    char lv_name[64];
    char vg_name[64];
    unsigned long long size_bytes;
    char lv_path[256];
    int is_snapshot;
    char origin[64];
} lv_info_t;

int lvm_pv_create(const char *device);
int lvm_vg_create(const char *vg_name, char **pvs, int pv_count);
int lvm_lv_create(const char *vg_name, const char *lv_name, unsigned long long size_mb);
int lvm_lv_extend(const char *vg_name, const char *lv_name, unsigned long long add_mb);
int lvm_snapshot_create(const char *vg_name, const char *origin_lv,
                        const char *snap_name, unsigned long long size_mb);

#endif
