1. Command Reference
RAID Commands
storage_cli raid create <array> --level=N --devices=d1,d2
storage_cli raid status <array>
storage_cli raid fail <array> <device>
storage_cli raid add <array> <device>

LVM Commands
storage_cli lvm pv-create <device>
storage_cli lvm vg-create <vg> <pv1> <pv2>
storage_cli lvm lv-create <vg> <lv> <size>

Filesystem Commands
storage_cli fs create <device> <ext4|xfs>
storage_cli fs mount <device> <mountpoint>
storage_cli fs check <device>

Backup Commands
storage_cli backup create <src> <dest> --type=full|incremental
storage_cli backup restore <id> <path>

Monitoring Commands
storage_cli monitor stats
storage_cli monitor history <device>

Performance Commands
storage_cli perf benchmark <device>
storage_cli perf tune <device> --scheduler=deadline

2. Configuration Guide

Socket path: /var/run/storage_mgr.sock

Shared memory: /dev/shm/storage_mgr_status

Message queue: /msgq_storage_mgr

Kernel module: /proc/storage_stats

Backups stored in /backup by default

3. Troubleshooting
Issue	Solution
Daemon not running	sudo ./storage_daemon
Kernel module fails	Check dmesg for errors
RAID commands fail	Ensure loop devices exist
“Permission denied”	Run commands with sudo
Backups incomplete	Verify snapshot size
4. FAQ

Q: Does it support Btrfs?
A: Yes, as an optional extended feature.

Q: Can I run it without root?
A: No, storage operations require root privileges.

Q: How do I view real-time performance?
A: ./storage_cli monitor stats
