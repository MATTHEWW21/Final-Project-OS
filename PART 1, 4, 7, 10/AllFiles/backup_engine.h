#ifndef BACKUP_ENGINE_H
#define BACKUP_ENGINE_H

#include <time.h>

typedef enum {
    BACKUP_FULL,
    BACKUP_INCREMENTAL,
    BACKUP_DIFFERENTIAL
} backup_type_t;

typedef struct {
    char backup_id[64];
    time_t timestamp;
    backup_type_t type;
    char source_path[256];
    char dest_path[256];
    unsigned long long size_bytes;
    char checksum[65];      // SHA256
    int success;
} backup_info_t;

int backup_create(const char *source, const char *dest, backup_type_t type);
int backup_create_snapshot(const char *vg_name, const char *lv_name,
                           const char *snapshot_name);
int backup_verify(const char *backup_id);
int backup_restore(const char *backup_id, const char *dest);
int backup_list(backup_info_t **backups, int *count);
int backup_cleanup(int keep_count);

#endif
