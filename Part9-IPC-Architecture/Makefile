CC=gcc
CFLAGS=-Wall

all: storage_daemon storage_cli

storage_daemon: ipc_server.c
	$(CC) ipc_server.c -o storage_daemon $(CFLAGS)

storage_cli: storage_cli.c
	$(CC) storage_cli.c -o storage_cli $(CFLAGS)

clean:
	rm -f storage_daemon storage_cli
