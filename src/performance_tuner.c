#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "performance_tuner.h"

int perf_set_scheduler(const char *device, const char *scheduler) {
    char path[256];
    snprintf(path, sizeof(path), 
        "/sys/block/%s/queue/scheduler", device + 5);

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "echo %s | sudo tee %s > /dev/null", scheduler, path);

    return system(cmd);
}

int perf_get_scheduler(const char *device, char *scheduler) {
    char path[256];
    snprintf(path, sizeof(path),
        "/sys/block/%s/queue/scheduler", device + 5);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    fscanf(f, "%s", scheduler);
    fclose(f);
    return 0;
}

int perf_set_readahead(const char *device, int size_kb) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
        "sudo blockdev --setra %d %s", size_kb, device);
    return system(cmd);
}

int perf_benchmark(const char *device, const char *output_file) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "sudo fio --name=bench --filename=%s "
        "--rw=randrw --bs=4k --size=512M "
        "--iodepth=16 --numjobs=1 --time_based "
        "--runtime=20 --group_reporting > %s",
        device, output_file);
    return system(cmd);
}

int perf_recommend(const char *device, workload_type_t workload,
                   tuning_profile_t *profile)
{
    switch (workload) {
        case WORKLOAD_DATABASE:
            strcpy(profile->scheduler, "deadline");
            profile->read_ahead_kb = 256;
            profile->queue_depth = 64;
            profile->vm_swappiness = 10;
            profile->vm_dirty_ratio = 10;
            break;

        case WORKLOAD_WEB_SERVER:
            strcpy(profile->scheduler, "cfq");
            profile->read_ahead_kb = 128;
            profile->queue_depth = 32;
            profile->vm_swappiness = 20;
            profile->vm_dirty_ratio = 15;
            break;

        case WORKLOAD_FILE_SERVER:
            strcpy(profile->scheduler, "bfq");
            profile->read_ahead_kb = 1024;
            profile->queue_depth = 128;
            profile->vm_swappiness = 30;
            profile->vm_dirty_ratio = 20;
            break;

        default:
            strcpy(profile->scheduler, "mq-deadline");
            profile->read_ahead_kb = 256;
            profile->queue_depth = 32;
            profile->vm_swappiness = 20;
            profile->vm_dirty_ratio = 10;
            break;
    }

    return 0;
}

int perf_apply_profile(const char *device, const tuning_profile_t *profile) {
    char cmd[256];

    // Scheduler
    perf_set_scheduler(device, profile->scheduler);

    // Read-ahead
    perf_set_readahead(device, profile->read_ahead_kb);

    // Queue depth
    snprintf(cmd, sizeof(cmd),
        "echo %d | sudo tee /sys/block/%s/queue/nr_requests",
        profile->queue_depth, device + 5);
    system(cmd);

    // Kernel params
    snprintf(cmd, sizeof(cmd),
        "sudo sysctl -w vm.swappiness=%d", profile->vm_swappiness);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
        "sudo sysctl -w vm.dirty_ratio=%d", profile->vm_dirty_ratio);
    system(cmd);

    return 0;
}
