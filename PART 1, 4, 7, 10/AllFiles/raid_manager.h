#ifndef RAID_MANAGER_H
#define RAID_MANAGER_H

typedef struct {
    char name[64];          // e.g., "/dev/md0"
    int raid_level;         // 0, 1, 5, 10
    int num_devices;
    char devices[16][64];   // Array of device paths
    char status[32];        // "active", "degraded", "failed"
    int num_failed;
} raid_array_t;

int raid_create(const char *array_name, int level, char **devices, int count);
int raid_monitor(raid_array_t *array);
int raid_add_disk(const char *array_name, const char *device);
int raid_fail_disk(const char *array_name, const char *device);
int raid_remove_disk(const char *array_name, const char *device);
int raid_stop(const char *array_name);

#endif
