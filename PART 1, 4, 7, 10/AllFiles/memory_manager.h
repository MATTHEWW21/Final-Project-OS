#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

typedef struct {
    unsigned long long total_kb;
    unsigned long long free_kb;
    unsigned long long available_kb;
    unsigned long long cached_kb;
    unsigned long long buffers_kb;
    unsigned long long swap_total_kb;
    unsigned long long swap_free_kb;
    float memory_pressure;  // 0.0 to 1.0
} memory_info_t;

int swap_create(const char *path, unsigned long long size_mb);
int swap_enable(const char *path, int priority);
int swap_disable(const char *path);

int memory_get_info(memory_info_t *info);
int memory_check_pressure(memory_info_t *info);

#endif
