#include <stdio.h>
#include <string.h>
#include "lvm_manager.h"
#include "storage_db.h"

unsigned long long MB_TO_BYTES(unsigned long long mb) {
    return mb * 1024ULL * 1024ULL;
}

int lvm_pv_create(const char *device) {
    if (PV_COUNT >= MAX_PVS) return -1;

    pv_info_t *pv = &PV_TABLE[PV_COUNT++];
    strcpy(pv->pv_name, device);
    pv->size_bytes = 10ULL * 1024ULL * 1024ULL * 1024ULL; // 10 GB fake size
    strcpy(pv->vg_name, "");

    printf("PV created: %s (10GB)\n", device);
    return 0;
}

int lvm_vg_create(const char *vg_name, char **pvs, int pv_count) {
    vg_info_t *vg = &VG_TABLE[VG_COUNT++];
    strcpy(vg->vg_name, vg_name);

    unsigned long long total_size = 0;

    for (int i=0; i<pv_count; i++) {
        for (int j=0; j<PV_COUNT; j++) {
            if (strcmp(PV_TABLE[j].pv_name, pvs[i]) == 0) {
                strcpy(PV_TABLE[j].vg_name, vg_name);
                total_size += PV_TABLE[j].size_bytes;
            }
        }
    }

    vg->size_bytes = total_size;
    vg->free_bytes = total_size;
    vg->pv_count = pv_count;
    vg->lv_count = 0;

    printf("VG %s created: %llu bytes\n", vg_name, total_size);
    return 0;
}

int lvm_lv_create(const char *vg_name, const char *lv_name, unsigned long long size_mb) {
    vg_info_t *vg = NULL;

    for (int i=0; i<VG_COUNT; i++) {
        if (strcmp(VG_TABLE[i].vg_name, vg_name) == 0) {
            vg = &VG_TABLE[i];
            break;
        }
    }
    if (!vg) return -1;

    unsigned long long size_bytes = MB_TO_BYTES(size_mb);
    if (vg->free_bytes < size_bytes) return -2;

    lv_info_t *lv = &LV_TABLE[LV_COUNT++];
    strcpy(lv->lv_name, lv_name);
    strcpy(lv->vg_name, vg_name);
    lv->size_bytes = size_bytes;
    lv->is_snapshot = 0;

    sprintf(lv->lv_path, "/dev/%s/%s", vg_name, lv_name);

    vg->free_bytes -= size_bytes;
    vg->lv_count++;

    printf("LV %s created (%llu bytes)\n", lv_name, size_bytes);
    return 0;
}

int lvm_lv_extend(const char *vg_name, const char *lv_name, unsigned long long add_mb) {
    vg_info_t *vg = NULL;

    for (int i=0; i<VG_COUNT; i++) {
        if (strcmp(VG_TABLE[i].vg_name, vg_name) == 0) {
            vg = &VG_TABLE[i];
            break;
        }
    }
    if (!vg) return -1;

    unsigned long long add_bytes = MB_TO_BYTES(add_mb);

    if (vg->free_bytes < add_bytes) return -2;

    for (int i=0; i<LV_COUNT; i++) {
        if (strcmp(LV_TABLE[i].lv_name, lv_name) == 0 &&
            strcmp(LV_TABLE[i].vg_name, vg_name) == 0)
        {
            LV_TABLE[i].size_bytes += add_bytes;
            vg->free_bytes -= add_bytes;

            printf("LV %s extended by %llu bytes\n", lv_name, add_bytes);
            return 0;
        }
    }
    return -3;
}

int lvm_snapshot_create(const char *vg_name, const char *origin_lv,
                        const char *snap_name, unsigned long long size_mb)
{
    lv_info_t *origin = NULL;

    for (int i=0; i<LV_COUNT; i++) {
        if (strcmp(LV_TABLE[i].lv_name, origin_lv) == 0 &&
            strcmp(LV_TABLE[i].vg_name, vg_name) == 0)
        {
            origin = &LV_TABLE[i];
            break;
        }
    }
    if (!origin) return -1;

    vg_info_t *vg = NULL;
    for (int i=0; i<VG_COUNT; i++) {
        if (strcmp(VG_TABLE[i].vg_name, vg_name) == 0) {
            vg = &VG_TABLE[i];
            break;
        }
    }
    if (!vg) return -2;

    unsigned long long size_bytes = MB_TO_BYTES(size_mb);
    if (vg->free_bytes < size_bytes) return -3;

    lv_info_t *snap = &LV_TABLE[LV_COUNT++];
    strcpy(snap->lv_name, snap_name);
    strcpy(snap->vg_name, vg_name);
    snap->size_bytes = size_bytes;
    snap->is_snapshot = 1;
    strcpy(snap->origin, origin_lv);

    sprintf(snap->lv_path, "/dev/%s/%s", vg_name, snap_name);

    vg->free_bytes -= size_bytes;
    vg->lv_count++;

    printf("Snapshot %s created from %s\n", snap_name, origin_lv);
    return 0;
}
