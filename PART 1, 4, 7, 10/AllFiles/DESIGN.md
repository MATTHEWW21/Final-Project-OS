# DESIGN

## 1. Architecture Overview

This partial implementation focuses on:

- `raid_manager`: wraps `mdadm` to manage software RAID arrays.
- `memory_manager`: manages swap files and reads virtual memory information from `/proc/meminfo`.
- `backup_engine`: creates simple tar-based backups and supports LVM snapshot creation.
- `storage_cli`: a simple CLI frontend that exposes RAID, swap, memory and backup operations.
- `storage_stats` kernel module: exposes basic storage statistics via `/proc/storage_stats`.

## 2. Components

### 2.1 RAID Manager

- Responsible for creating and managing software RAID arrays via `mdadm`.
- Uses a thin C wrapper (`system()` calls for now) around typical `mdadm` commands.
- Reads status from `/proc/mdstat`.

### 2.2 Memory Manager

- Creates swap files using `fallocate`, `chmod` and `mkswap`.
- Enables/disables swap using `swapon` / `swapoff`.
- Parses `/proc/meminfo` and computes a simple `memory_pressure` metric.

### 2.3 Backup Engine

- Uses `tar` to create full backups of a given directory.
- Provides a helper for creating LVM snapshots (via `lvcreate -s`).
- Includes placeholders for listing backups and cleanup policies.

### 2.4 Kernel Module (`storage_stats`)

- Simple kernel module that registers `/proc/storage_stats`.
- Exposes a static `storage_stats_t` structure.
- Future work: integrate with user-space daemon or block layer hooks.

## 3. Data Structures

- `raid_array_t` – describes a RAID array and high-level status.
- `memory_info_t` – captures memory and swap usage from `/proc/meminfo`.
- `backup_info_t` – describes a backup entry (not fully implemented here).
- `storage_stats_t` – holds basic I/O counters inside the kernel module.

## 4. Design Decisions

- Initial implementation uses `system()` for simplicity. This can be replaced by `fork()+execvp()` later.
- Separate headers in `include/` to keep APIs clean and testable.
- Kernel module kept minimal to reduce complexity and risk while developing.
