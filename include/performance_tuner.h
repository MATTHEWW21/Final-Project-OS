#ifndef PERFORMANCE_TUNER_H
#define PERFORMANCE_TUNER_H

typedef enum {
    WORKLOAD_DATABASE,
    WORKLOAD_WEB_SERVER,
    WORKLOAD_FILE_SERVER,
    WORKLOAD_GENERAL
} workload_type_t;

typedef struct {
    char scheduler[32];
    int read_ahead_kb;
    int queue_depth;
    int vm_swappiness;
    int vm_dirty_ratio;
} tuning_profile_t;

// Scheduler
int perf_set_scheduler(const char *device, const char *scheduler);
int perf_get_scheduler(const char *device, char *scheduler);

// Block tuning
int perf_set_readahead(const char *device, int size_kb);

// Benchmarking
int perf_benchmark(const char *device, const char *output_file);

// Recommendations
int perf_recommend(const char *device, workload_type_t workload,
                   tuning_profile_t *profile);

int perf_apply_profile(const char *device, const tuning_profile_t *profile);

#endif
