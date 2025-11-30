typedef struct {
    char device[64];
    unsigned long long reads;
    unsigned long long writes;
    unsigned long long read_bytes;
    unsigned long long write_bytes;
    double avg_read_latency_ms;
    double avg_write_latency_ms;
    int queue_depth;
} device_stats_t;

typedef struct {
    time_t timestamp;
    double iops;
    double throughput_mbs;
    double latency_ms;
    int active_requests;
} performance_sample_t;

int monitor_get_device_stats(const char *device, device_stats_t *stats);
int monitor_get_io_stats(const char *device, device_stats_t *stats);
int monitor_track_performance(const char *device, performance_sample_t *sample);
int monitor_list_open_files(const char *mount_point, char ***files, int *count);
