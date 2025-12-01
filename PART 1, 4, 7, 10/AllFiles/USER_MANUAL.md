# USER MANUAL

## 1. Installation

On Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential mdadm lvm2 \
    linux-headers-$(uname -r) tar
```

Clone or copy this project, then:

```bash
make
```

For the kernel module:

```bash
cd kernel_module
make
```

## 2. RAID Commands

```bash
sudo ./storage_cli raid create /dev/md0 --level=1 --devices=/dev/loop0,/dev/loop1
sudo ./storage_cli raid status /dev/md0
sudo ./storage_cli raid fail /dev/md0 /dev/loop0
sudo ./storage_cli raid add /dev/md0 /dev/loop2
sudo ./storage_cli raid remove /dev/md0 /dev/loop0
sudo ./storage_cli raid stop /dev/md0
```

> WARNING: Only use loop devices or test disks in a VM.

## 3. Swap & Memory

```bash
sudo ./storage_cli swap create /swapfile 2048M
sudo ./storage_cli swap enable /swapfile --priority=10
./storage_cli memory status
sudo ./storage_cli swap disable /swapfile
```

## 4. Backup

```bash
sudo ./storage_cli backup create /mnt/data /backup/data
sudo ./storage_cli backup restore /backup/data/backup-YYYYMMDD-HHMMSS.tar.gz /restore/test
```

## 5. Kernel Module

```bash
cd kernel_module
sudo insmod storage_stats.ko
cat /proc/storage_stats
sudo rmmod storage_stats
dmesg | tail
```

## 6. Troubleshooting

- If `mdadm` commands fail, check:
  - Are you running as root (`sudo`)?
  - Are the loop devices set up and not in use?
- If the kernel module fails to build:
  - Ensure `linux-headers-$(uname -r)` is installed.
- Always test inside a virtual machine.
