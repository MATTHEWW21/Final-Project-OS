#include <stdio.h>
#include <string.h>
#include "storage_db.h"

pv_info_t PV_TABLE[MAX_PVS];
vg_info_t VG_TABLE[MAX_VGS];
lv_info_t LV_TABLE[MAX_LVS];

int PV_COUNT = 0;
int VG_COUNT = 0;
int LV_COUNT = 0;

void db_init() {
    PV_COUNT = 0;
    VG_COUNT = 0;
    LV_COUNT = 0;
}

void db_save() {}
void db_load() {}
