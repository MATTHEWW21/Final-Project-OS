#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#include <time.h>

typedef struct {
    char user[64];
    char permissions[16];   // "rwx", "r-x", etc.
} acl_entry_t;

typedef struct {
    char device[256];
    char name[64];
    int is_open;
    char dm_name[64];       // e.g., "luks-data"
    char pass_hash[128];    // simulated passphrase storage (not real LUKS)
} encrypted_volume_t;

/* ACL functions */
int acl_set(const char *path, const char *user, const char *perms, int recursive, int is_default);
int acl_get(const char *path, acl_entry_t *entries, int *count);
int acl_remove(const char *path, const char *user, int recursive);

/* Attributes (immutable / append-only) */
int attr_set(const char *path, const char *flags);   // flags like "+i", "+a", "-i", "-a"
int attr_get(const char *path, char *outflags, int outsz);

/* Simulated LUKS functions */
int luks_format(const char *device, const char *name, const char *passphrase);
int luks_open(const char *device_or_name, const char *name, const char *passphrase);
int luks_close(const char *name);

/* Audit logging (tamper-evident chain) */
int audit_log(const char *operation, const char *user, const char *details);
int audit_show(int last_n); // show last_n entries (0 = all)

/* Helpers */
int security_init();

#endif
