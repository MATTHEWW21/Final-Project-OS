typedef enum {
    FS_TYPE_EXT4,
    FS_TYPE_XFS,
    FS_TYPE_BTRFS
} fs_type_t;

typedef struct {
    char device[256];
    char mount_point[256];
    fs_type_t type;
    char options[256];
    unsigned long long total_bytes;
    unsigned long long used_bytes;
    unsigned long long available_bytes;
} fs_info_t;

int fs_create(const char *device, fs_type_t type, const char *label);
int fs_mount(const char *device, const char *mount_point, 
             fs_type_t type, const char *options);
int fs_unmount(const char *mount_point, int force);
int fs_check(const char *device, fs_type_t type);
int fs_resize(const char *device, fs_type_t type, 
              unsigned long long new_size_mb);
int fs_get_info(const char *mount_point, fs_info_t *info);
