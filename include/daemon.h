#ifndef DAEMON_H
#define DAEMON_H

#include <sys/types.h>
#include <time.h>

typedef struct {
    pid_t pid;
    int status;          // 0 = idle, 1 = busy
    time_t started;
    char task[256];
} worker_t;

int daemon_init(void);
int daemon_create_pidfile(const char *path);
int daemon_setup_signals(void);
void daemon_signal_handler(int sig);

int daemon_spawn_worker(void (*worker_func)(void *), void *arg);
int daemon_monitor_workers(worker_t *workers, int count);

void daemon_shutdown(void);

#endif
