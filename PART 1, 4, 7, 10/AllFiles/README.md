# Enterprise Storage Manager (Partial Implementation)

This repository contains a partial implementation of the **Enterprise Storage Manager** project, focusing on:

- RAID management (Part 1)
- Virtual memory / swap management (Part 4)
- Backup system (Part 7)
- Kernel module for storage statistics (Part 10)
- Documentation skeleton (Part 13)

## Project Structure

- `src/` – Core implementation (`raid_manager`, `memory_manager`, `backup_engine`)
- `include/` – Public headers
- `cli/` – Command line client (`storage_cli`)
- `kernel_module/` – `storage_stats` kernel module
- `docs/` – Documentation (skeleton)

## Build Instructions

Requirements (Ubuntu):

```bash
sudo apt update
sudo apt install -y build-essential mdadm lvm2 \
    linux-headers-$(uname -r) tar
```

Build user-space tools:

```bash
make
```

Build kernel module:

```bash
cd kernel_module
make
```

## Usage Examples

### RAID

```bash
sudo ./storage_cli raid create /dev/md0 --level=1 --devices=/dev/loop0,/dev/loop1
sudo ./storage_cli raid status /dev/md0
sudo ./storage_cli raid fail /dev/md0 /dev/loop0
sudo ./storage_cli raid add /dev/md0 /dev/loop2
sudo ./storage_cli raid stop /dev/md0
```

### Swap / Memory

```bash
sudo ./storage_cli swap create /swapfile 2048M
sudo ./storage_cli swap enable /swapfile --priority=10
./storage_cli memory status
sudo ./storage_cli swap disable /swapfile
```

### Backup

```bash
sudo ./storage_cli backup create /mnt/data /backup/data
sudo ./storage_cli backup restore /backup/data/backup-YYYYMMDD-HHMMSS.tar.gz /restore/test
```

### Kernel Module

```bash
cd kernel_module
sudo insmod storage_stats.ko
cat /proc/storage_stats
sudo rmmod storage_stats
dmesg | tail
```
