#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/daemon.h"

static const char *PID_FILE = "/var/run/storage_mgr.pid";
static volatile sig_atomic_t running = 1;

/* ------------------------------------------------------
 * Create PID file
 * ------------------------------------------------------ */
int daemon_create_pidfile(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f, "%d\n", getpid());
    fclose(f);

    return 0;
}

/* ------------------------------------------------------
 * SIGNAL HANDLING
 * ------------------------------------------------------ */

void daemon_signal_handler(int sig) {
    switch (sig) {
        case SIGTERM:
            running = 0;
            break;

        case SIGHUP:
            // Reload config
            printf("Daemon received SIGHUP: reload config\n");
            break;

        case SIGCHLD:
            // Reap zombie workers
            while (waitpid(-1, NULL, WNOHANG) > 0);
            break;

        case SIGUSR1:
            // Dump status
            printf("Daemon received SIGUSR1: printing diagnostic info\n");
            break;
    }
}

int daemon_setup_signals(void) {
    struct sigaction sa;
    sa.sa_handler = daemon_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &sa, NULL) == -1) return -1;
    if (sigaction(SIGHUP, &sa, NULL) == -1) return -1;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) return -1;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) return -1;

    return 0;
}

/* ------------------------------------------------------
 * DAEMON INIT (double-fork)
 * ------------------------------------------------------ */

int daemon_init(void) {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(0);   // Parent exits

    // Child continues
    if (setsid() < 0) return -1;

    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(0); // First child exits

    // Second child = daemon

    umask(0);
    chdir("/");

    // Close all FDs
    for (int fd = 0; fd < 1024; fd++) {
        close(fd);
    }

    // Redirect stdio
    int fd0 = open("/dev/null", O_RDWR);
    dup2(fd0, 0);
    dup2(fd0, 1);
    dup2(fd0, 2);

    // Create PID file
    if (daemon_create_pidfile(PID_FILE) != 0) {
        return -1;
    }

    // Install signals
    if (daemon_setup_signals() != 0) {
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------
 * WORKERS
 * ------------------------------------------------------ */

int daemon_spawn_worker(void (*worker_func)(void *), void *arg) {
    pid_t pid = fork();

    if (pid < 0) return -1;

    if (pid == 0) {
        // Worker process

        // Limit resources
        struct rlimit rl;
        rl.rlim_cur = 1024 * 1024 * 512; // 512MB
        rl.rlim_max = 1024 * 1024 * 512;
        setrlimit(RLIMIT_AS, &rl);

        worker_func(arg);
        exit(0);
    }

    return pid;
}

int daemon_monitor_workers(worker_t *workers, int count) {
    for (int i = 0; i < count; i++) {
        if (workers[i].pid <= 0) continue;

        int status = 0;
        pid_t r = waitpid(workers[i].pid, &status, WNOHANG);

        if (r > 0) {
            // Worker died -> restart
            printf("Worker %d FAILED, restarting\n", workers[i].pid);

            workers[i].pid = daemon_spawn_worker(NULL, NULL);
            workers[i].status = 0;
            workers[i].started = time(NULL);
            strcpy(workers[i].task, "restarted");
        }
    }
    return 0;
}

/* ------------------------------------------------------
 * DAEMON SHUTDOWN
 * ------------------------------------------------------ */

void daemon_shutdown(void) {
    unlink(PID_FILE);
    running = 0;
}
