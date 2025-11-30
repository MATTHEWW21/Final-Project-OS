#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include "ipc_server.h"

#define SOCK_PATH "/var/run/storage_mgr.sock"
#define SHM_PATH "/dev/shm/storage_mgr_status"
#define MSGQ_KEY 0x1234

struct msgbuf {
    long mtype;
    char mtext[256];
};

int create_shared_memory() {
    int fd = open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (fd < 0) return -1;

    ftruncate(fd, 4096);
    char *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    strcpy(ptr, "STATUS_OK");
    return 0;
}

int msgq_init() {
    return msgget(MSGQ_KEY, IPC_CREAT | 0666);
}

int ipc_server_init(const char *socket_path) {
    unlink(socket_path);

    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) return -1;

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return -1;
    if (listen(fd, 5) < 0) return -1;

    return fd;
}

void handle_request(int client, request_t *req) {
    response_t resp = {0};
    resp.request_id = req->request_id;

    switch (req->command) {
        case CMD_STATUS:
            strcpy(resp.data, "Daemon OK. IPC online.");
            resp.data_size = strlen(resp.data);
            resp.status = 0;
            break;

        case CMD_MONITOR_STATS:
            strcpy(resp.data, "Stats: CPU=10%, IO=3MB/s.");
            resp.data_size = strlen(resp.data);
            resp.status = 0;
            break;

        default:
            resp.status = -1;
            strcpy(resp.error_msg, "Unknown command");
            break;
    }

    write(client, &resp, sizeof(resp));
}

int main() {
    printf("[+] Starting IPC daemon...\n");

    create_shared_memory();
    msgq_init();

    int server_fd = ipc_server_init(SOCK_PATH);
    if (server_fd < 0) {
        perror("server_init");
        exit(1);
    }

    printf("[+] Listening on %s\n", SOCK_PATH);

    fd_set readfds;
    int clients[10] = {0};

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int maxfd = server_fd;

        for (int i = 0; i < 10; i++) {
            if (clients[i] > 0) {
                FD_SET(clients[i], &readfds);
                if (clients[i] > maxfd)
                    maxfd = clients[i];
            }
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            int newfd = accept(server_fd, NULL, NULL);

            for (int i = 0; i < 10; i++) {
                if (clients[i] == 0) {
                    clients[i] = newfd;
                    break;
                }
            }
        }

        for (int i = 0; i < 10; i++) {
            if (clients[i] > 0 && FD_ISSET(clients[i], &readfds)) {
                request_t req = {0};
                int n = read(clients[i], &req, sizeof(req));

                if (n <= 0) {
                    close(clients[i]);
                    clients[i] = 0;
                } else {
                    handle_request(clients[i], &req);
                }
            }
        }
    }

    return 0;
}
