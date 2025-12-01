#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "raid_manager.h"
#include "memory_manager.h"
#include "backup_engine.h"

static void usage(void) {
    printf("Usage:\n");
    printf("  storage_cli raid create <array> --level=N --devices=dev1,dev2,...\n");
    printf("  storage_cli raid fail <array> <device>\n");
    printf("  storage_cli raid add <array> <device>\n");
    printf("  storage_cli raid remove <array> <device>\n");
    printf("  storage_cli raid stop <array>\n");
    printf("  storage_cli raid status <array>\n");
    printf("\n");
    printf("  storage_cli swap create <path> <size> (e.g. 2G or 2048M)\n");
    printf("  storage_cli swap enable <path> --priority=N\n");
    printf("  storage_cli swap disable <path>\n");
    printf("\n");
    printf("  storage_cli memory status\n");
    printf("\n");
    printf("  storage_cli backup create <source> <dest>\n");
    printf("  storage_cli backup restore <backup_path> <dest>\n");
}

/* Parse size like "2G" or "2048M" to MB */
static unsigned long long parse_size_mb(const char *s) {
    char *end = NULL;
    unsigned long long val = strtoull(s, &end, 10);
    if (*end == 'G' || *end == 'g') {
        return val * 1024ULL;
    } else if (*end == 'M' || *end == 'm' || *end == '\0') {
        return val;
    }
    return val;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }

    /* RAID subcommand */
    if (strcmp(argv[1], "raid") == 0) {
        if (argc < 3) {
            usage();
            return 1;
        }

        const char *cmd = argv[2];

        if (strcmp(cmd, "create") == 0) {
            if (argc < 6) {
                usage();
                return 1;
            }
            const char *array_name = argv[3];
            int level = 0;
            char *devices_str = NULL;

            for (int i = 4; i < argc; i++) {
                if (strncmp(argv[i], "--level=", 8) == 0) {
                    level = atoi(argv[i] + 8);
                } else if (strncmp(argv[i], "--devices=", 10) == 0) {
                    devices_str = argv[i] + 10;
                }
            }
            if (!devices_str) {
                fprintf(stderr, "Missing devices\n");
                return 1;
            }

            char *devs[16];
            int count = 0;
            char *tmp = strdup(devices_str);
            char *token = strtok(tmp, ",");
            while (token && count < 16) {
                devs[count++] = token;
                token = strtok(NULL, ",");
            }

            int ret = raid_create(array_name, level, devs, count);
            free(tmp);
            return ret == 0 ? 0 : 1;
        } else if (strcmp(cmd, "fail") == 0 && argc == 5) {
            return raid_fail_disk(argv[3], argv[4]) == 0 ? 0 : 1;
        } else if (strcmp(cmd, "add") == 0 && argc == 5) {
            return raid_add_disk(argv[3], argv[4]) == 0 ? 0 : 1;
        } else if (strcmp(cmd, "remove") == 0 && argc == 5) {
            return raid_remove_disk(argv[3], argv[4]) == 0 ? 0 : 1;
        } else if (strcmp(cmd, "stop") == 0 && argc == 4) {
            return raid_stop(argv[3]) == 0 ? 0 : 1;
        } else if (strcmp(cmd, "status") == 0 && argc == 4) {
            raid_array_t arr;
            memset(&arr, 0, sizeof(arr));
            strncpy(arr.name, argv[3], sizeof(arr.name) - 1);
            if (raid_monitor(&arr) == 0) {
                printf("Array: %s\n", arr.name);
                printf("Status: %s\n", arr.status);
                printf("Failed disks: %d\n", arr.num_failed);
                return 0;
            }
            return 1;
        } else {
            usage();
            return 1;
        }
    }

    /* SWAP subcommand */
    if (strcmp(argv[1], "swap") == 0) {
        if (argc < 3) {
            usage();
            return 1;
        }
        const char *cmd = argv[2];

        if (strcmp(cmd, "create") == 0) {
            if (argc < 5) {
                usage();
                return 1;
            }
            const char *path = argv[3];
            unsigned long long size_mb = parse_size_mb(argv[4]);
            if (swap_create(path, size_mb) == 0) {
                printf("Swap created at %s (%llu MB)\n", path, size_mb);
                return 0;
            }
            return 1;
        } else if (strcmp(cmd, "enable") == 0) {
            if (argc < 4) {
                usage();
                return 1;
            }
            const char *path = argv[3];
            int priority = 0;
            for (int i = 4; i < argc; i++) {
                if (strncmp(argv[i], "--priority=", 11) == 0) {
                    priority = atoi(argv[i] + 11);
                }
            }
            if (swap_enable(path, priority) == 0) {
                printf("Swap enabled on %s with priority %d\n", path, priority);
                return 0;
            }
            return 1;
        } else if (strcmp(cmd, "disable") == 0) {
            if (argc < 4) {
                usage();
                return 1;
            }
            const char *path = argv[3];
            if (swap_disable(path) == 0) {
                printf("Swap disabled on %s\n", path);
                return 0;
            }
            return 1;
        } else {
            usage();
            return 1;
        }
    }

    /* MEMORY subcommand */
    if (strcmp(argv[1], "memory") == 0) {
        if (argc < 3) {
            usage();
            return 1;
        }
        const char *cmd = argv[2];
        if (strcmp(cmd, "status") == 0) {
            memory_info_t info;
            if (memory_get_info(&info) != 0) {
                return 1;
            }
            memory_check_pressure(&info);

            printf("MemTotal:     %llu kB\n", info.total_kb);
            printf("MemFree:      %llu kB\n", info.free_kb);
            printf("MemAvailable: %llu kB\n", info.available_kb);
            printf("Cached:       %llu kB\n", info.cached_kb);
            printf("Buffers:      %llu kB\n", info.buffers_kb);
            printf("SwapTotal:    %llu kB\n", info.swap_total_kb);
            printf("SwapFree:     %llu kB\n", info.swap_free_kb);
            printf("MemoryPressure: %.2f\n", info.memory_pressure);

            return 0;
        } else {
            usage();
            return 1;
        }
    }

    /* BACKUP subcommand */
    if (strcmp(argv[1], "backup") == 0) {
        if (argc < 3) {
            usage();
            return 1;
        }
        const char *cmd = argv[2];

        if (strcmp(cmd, "create") == 0 && argc == 5) {
            const char *source = argv[3];
            const char *dest   = argv[4];
            return backup_create(source, dest, BACKUP_FULL) == 0 ? 0 : 1;
        } else if (strcmp(cmd, "restore") == 0 && argc == 5) {
            const char *backup_path = argv[3];
            const char *dest        = argv[4];
            return backup_restore(backup_path, dest) == 0 ? 0 : 1;
        } else {
            usage();
            return 1;
        }
    }

    usage();
    return 1;
}
