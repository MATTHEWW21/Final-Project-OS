# API REFERENCE

## raid_manager.h

### `int raid_create(const char *array_name, int level, char **devices, int count);`
Creates a RAID array using `mdadm`.

- `array_name`: e.g., `/dev/md0`
- `level`: RAID level (0, 1, 5, 10)
- `devices`: array of device paths (e.g., `/dev/loop0`)
- `count`: number of devices
- Returns: `0` on success, `<0` on error.

### `int raid_monitor(raid_array_t *array);`
Reads `/proc/mdstat` and updates `array->status` and `array->num_failed`.

### `int raid_add_disk(const char *array_name, const char *device);`
Adds a device to an existing array via `mdadm --add`.

### `int raid_fail_disk(const char *array_name, const char *device);`
Marks a device as failed via `mdadm --fail`.

### `int raid_remove_disk(const char *array_name, const char *device);`
Removes a device from an array via `mdadm --remove`.

### `int raid_stop(const char *array_name);`
Stops an array via `mdadm --stop`.

---

## memory_manager.h

### `int swap_create(const char *path, unsigned long long size_mb);`
Creates a swap file at `path` with size `size_mb` using `fallocate`, `chmod`, and `mkswap`.

### `int swap_enable(const char *path, int priority);`
Enables swap with `swapon -p priority path`.

### `int swap_disable(const char *path);`
Disables swap with `swapoff path`.

### `int memory_get_info(memory_info_t *info);`
Parses `/proc/meminfo` and fills `memory_info_t` fields.

### `int memory_check_pressure(memory_info_t *info);`
Computes a simple `memory_pressure` metric between `0.0` and `1.0`.

---

## backup_engine.h

### `int backup_create(const char *source, const char *dest, backup_type_t type);`
Creates a tarball backup from `source` into `dest` using `tar -czf`.

### `int backup_create_snapshot(const char *vg_name, const char *lv_name, const char *snapshot_name);`
Creates an LVM snapshot with `lvcreate -s`.

### `int backup_verify(const char *backup_path);`
Very simple verification that checks whether `backup_path` exists.

### `int backup_restore(const char *backup_path, const char *dest);`
Restores a tarball backup into destination directory `dest`.

The following are placeholders for future work:

- `int backup_list(backup_info_t **backups, int *count);`
- `int backup_cleanup(int keep_count);`
