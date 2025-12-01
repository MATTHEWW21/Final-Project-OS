#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "memory_manager.h"

static int run_cmd(const char *cmd) {
    int ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }
    if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
        return 0;
    }
    fprintf(stderr, "Command failed: %s\n", cmd);
    return -1;
}

int swap_create(const char *path, unsigned long long size_mb) {
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "fallocate -l %llum %s", size_mb, path);
    if (run_cmd(cmd) != 0) {
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "chmod 600 %s", path);
    if (run_cmd(cmd) != 0) {
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "mkswap %s", path);
    if (run_cmd(cmd) != 0) {
        return -1;
    }

    return 0;
}

int swap_enable(const char *path, int priority) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "swapon -p %d %s", priority, path);
    return run_cmd(cmd);
}

int swap_disable(const char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "swapoff %s", path);
    return run_cmd(cmd);
}

int memory_get_info(memory_info_t *info) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        perror("fopen /proc/meminfo");
        return -1;
    }

    char key[64];
    unsigned long long value;
    char unit[16];

    memset(info, 0, sizeof(*info));

    while (fscanf(f, "%63s %llu %15s\n", key, &value, unit) == 3) {
        if (strcmp(key, "MemTotal:") == 0) {
            info->total_kb = value;
        } else if (strcmp(key, "MemFree:") == 0) {
            info->free_kb = value;
        } else if (strcmp(key, "MemAvailable:") == 0) {
            info->available_kb = value;
        } else if (strcmp(key, "Cached:") == 0) {
            info->cached_kb = value;
        } else if (strcmp(key, "Buffers:") == 0) {
            info->buffers_kb = value;
        } else if (strcmp(key, "SwapTotal:") == 0) {
            info->swap_total_kb = value;
        } else if (strcmp(key, "SwapFree:") == 0) {
            info->swap_free_kb = value;
        }
    }

    fclose(f);
    return 0;
}

int memory_check_pressure(memory_info_t *info) {
    if (!info) return -1;

    if (info->total_kb == 0) {
        info->memory_pressure = 0.0f;
        return 0;
    }

    double available = (double)info->available_kb;
    double total = (double)info->total_kb;

    double pressure = 1.0 - (available / total);
    if (pressure < 0.0) pressure = 0.0;
    if (pressure > 1.0) pressure = 1.0;

    info->memory_pressure = (float)pressure;
    return 0;
}
