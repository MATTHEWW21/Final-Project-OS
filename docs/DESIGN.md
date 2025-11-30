System Architecture

The system uses a client–server design:

storage_daemon: central controller for all storage operations.

storage_cli: user-facing command-line interface.

Kernel module: provides low-level performance stats via /proc/storage_stats.

IPC layer: UNIX sockets, shared memory, and message queues.

Component Interactions

The CLI sends commands to the daemon via UNIX domain sockets.

The daemon dispatches requests to subsystem managers (RAID, LVM, FS, Backup).

Real-time status is stored in shared memory for quick access.

Long-running jobs (backup, rebuild) are handled through worker threads and message queues.

Kernel module provides stats read by the monitor engine.

Data Structures
Example
typedef struct {
    uint32_t version;
    uint32_t request_id;
    command_type_t command;
    uint32_t payload_size;
    char payload[4096];
} request_t;

Subsystem Structures

raid_array_t: RAID metadata

vg_info_t / lv_info_t: LVM structures

fs_info_t: filesystem and mount info

backup_info_t: backup records

device_stats_t: performance data

Design Decisions

UNIX sockets instead of TCP for faster local communication.

Shared memory for real-time statistics.

Message queues for asynchronous tasks.

Kernel module to expose low-level stats unavailable from userspace alone.

Modular subsystem design for easier maintenance and testing.

Trade-offs

High modularity increases code size but improves readability.

Using system tools (mdadm, LVM) simplifies development but reduces abstraction level.

Custom binary protocol improves performance but adds complexity.
