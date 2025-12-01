#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "raid_manager.h"

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

int raid_create(const char *array_name, int level, char **devices, int count) {
    char cmd[512] = {0};
    char devs[256] = {0};

    for (int i = 0; i < count; i++) {
        strcat(devs, " ");
        strcat(devs, devices[i]);
    }

    snprintf(cmd, sizeof(cmd),
             "mdadm --create %s --level=%d --raid-devices=%d%s",
             array_name, level, count, devs);

    return run_cmd(cmd);
}

int raid_monitor(raid_array_t *array) {
    FILE *f = fopen("/proc/mdstat", "r");
    if (!f) {
        perror("fopen /proc/mdstat");
        return -1;
    }

    char line[256];
    int found = 0;
    const char *name = array->name;

    const char *mdname = NULL;
    if (strncmp(name, "/dev/", 5) == 0) {
        mdname = name + 5; // skip "/dev/"
    } else {
        mdname = name;
    }

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, mdname)) {
            found = 1;
            if (strstr(line, "[UU]")) {
                strcpy(array->status, "active");
                array->num_failed = 0;
            } else if (strstr(line, "_U") || strstr(line, "U_")) {
                strcpy(array->status, "degraded");
                array->num_failed = 1;
            } else {
                strcpy(array->status, "unknown");
            }
            break;
        }
    }
    fclose(f);

    if (!found) {
        strcpy(array->status, "stopped");
    }

    return 0;
}

int raid_add_disk(const char *array_name, const char *device) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mdadm %s --add %s", array_name, device);
    return run_cmd(cmd);
}

int raid_fail_disk(const char *array_name, const char *device) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mdadm %s --fail %s", array_name, device);
    return run_cmd(cmd);
}

int raid_remove_disk(const char *array_name, const char *device) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mdadm %s --remove %s", array_name, device);
    return run_cmd(cmd);
}

int raid_stop(const char *array_name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mdadm --stop %s", array_name);
    return run_cmd(cmd);
}
