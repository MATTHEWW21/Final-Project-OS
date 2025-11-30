Public Structures
request_t

Defines the binary protocol for client-server messages.

response_t

Returned by the daemon with status code and optional data.

Public Functions (Examples)
int raid_create(const char *array, int level, char **devices, int count);

Parameters:

array: name of RAID device

level: RAID level

devices: list of devices

count: number of devices

Returns:
0 on success, negative on error.

int fs_mount(const char *device, const char *mount, fs_type_t type, const char *options);
int backup_create(const char *source, const char *dest, backup_type_t type);
int monitor_get_device_stats(const char *device, device_stats_t *stats);
Error Codes
Code	Meaning
0	Success
-1	Invalid arguments
-2	System call failure
-3	Device not found
-4	Operation not permitted
-5	IPC communication error
