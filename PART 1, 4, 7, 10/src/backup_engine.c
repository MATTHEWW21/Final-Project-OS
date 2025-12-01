#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#include "backup_engine.h"

static int run_cmd(const char *cmd) {
    int ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }
    if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
        return 0;
    fprintf(stderr, "Command failed: %s\n", cmd);
    return -1;
}

int backup_create(const char *source, const char *dest, backup_type_t type) {
    (void)type; /* currently only FULL using tar */
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char backup_id[64];
    char cmd[512];

    strftime(backup_id, sizeof(backup_id), "backup-%Y%m%d-%H%M%S.tar.gz", tm);

    snprintf(cmd, sizeof(cmd),
             "mkdir -p '%s' && cd '%s' && "
             "tar -czf '%s/%s' .",
             dest, source, dest, backup_id);

    return run_cmd(cmd);
}

int backup_create_snapshot(const char *vg_name, const char *lv_name,
                           const char *snapshot_name) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "lvcreate -L 1G -s -n %s /dev/%s/%s",
             snapshot_name, vg_name, lv_name);
    return run_cmd(cmd);
}

int backup_verify(const char *backup_path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "test -f '%s'", backup_path);
    return run_cmd(cmd);
}

int backup_restore(const char *backup_path, const char *dest) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "mkdir -p '%s' && tar -xzf '%s' -C '%s'",
             dest, backup_path, dest);
    return run_cmd(cmd);
}

int backup_list(backup_info_t **backups, int *count) {
    (void)backups;
    (void)count;
    fprintf(stderr, "backup_list: not implemented yet\n");
    return -1;
}

int backup_cleanup(int keep_count) {
    (void)keep_count;
    fprintf(stderr, "backup_cleanup: not implemented yet\n");
    return -1;
}
