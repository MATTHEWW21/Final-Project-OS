CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRCS = \
    src/raid_manager.c \
    src/memory_manager.c \
    src/backup_engine.c

OBJS = $(SRCS:.c=.o)

all: storage_cli

storage_cli: $(OBJS) cli/storage_cli.c
	$(CC) $(CFLAGS) -o storage_cli cli/storage_cli.c $(OBJS)

clean:
	rm -f $(OBJS) storage_cli
