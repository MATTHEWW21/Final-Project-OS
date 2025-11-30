#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include "ipc_server.h"

#define SOCK_PATH "/var/run/storage_mgr.sock"

int main(int argc, char *argv[]) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    request_t req = {0};
    req.version = 1;
    req.request_id = 100;

    if (argc > 1 && strcmp(argv[1], "stats") == 0)
        req.command = CMD_MONITOR_STATS;
    else
        req.command = CMD_STATUS;

    write(fd, &req, sizeof(req));

    response_t resp = {0};
    read(fd, &resp, sizeof(resp));

    printf("Reply: %s\n", resp.data);
    return 0;
}
