#ifndef STORAGE_DB_H
#define STORAGE_DB_H

#include "lvm_manager.h"

#define MAX_PVS 32
#define MAX_VGS 32
#define MAX_LVS 128

extern pv_info_t PV_TABLE[MAX_PVS];
extern vg_info_t VG_TABLE[MAX_VGS];
extern lv_info_t LV_TABLE[MAX_LVS];

extern int PV_COUNT;
extern int VG_COUNT;
extern int LV_COUNT;

void db_init();
void db_save();
void db_load();

#endif
