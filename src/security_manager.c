#define _POSIX_C_SOURCE 200809L
#include "security_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdint.h>
#include <time.h>

#define SEC_DIR ".mini_lvm/security"
#define ACL_DIR SEC_DIR"/acls"
#define ATTR_DIR SEC_DIR"/attrs"
#define LUKS_DIR SEC_DIR"/luks"
#define AUDIT_FILE SEC_DIR"/audit.log"
#define AUDIT_SIG SEC_DIR"/audit.sig"
#define MAPPER_DIR ".mini_lvm/dev/mapper"

static void ensure_dir(const char *d) {
    struct stat st;
    if (stat(d, &st) == -1) {
        mkdir(d, 0755);
    }
}

static void ensure_security_dirs() {
    ensure_dir(".mini_lvm");
    ensure_dir(".mini_lvm/dev");
    ensure_dir(".mini_lvm/dev/mapper");
    ensure_dir(SEC_DIR);
    ensure_dir(ACL_DIR);
    ensure_dir(ATTR_DIR);
    ensure_dir(LUKS_DIR);
}

/* --- simple filename sanitizer: replace '/' with '!' and ':' with '_' --- */
static void path_to_safe(const char *path, char *out, size_t outsz) {
    size_t j=0;
    for (size_t i=0; path[i] && j+1<outsz; ++i) {
        char c = path[i];
        if (c == '/') { out[j++] = '!'; continue; }
        if (c == ':') { out[j++] = '_'; continue; }
        if (c == ' ') { out[j++] = '_'; continue; }
        out[j++] = c;
    }
    out[j] = 0;
}

/* --- AUDIT: simple FNV-1a 64-bit rolling hash for tamper evidence --- */
static uint64_t fnv1a_hash64(const void *data, size_t len) {
    const uint8_t *ptr = (const uint8_t*)data;
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i=0;i<len;i++) {
        hash ^= (uint64_t)ptr[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* read last signature (if exists) */
static uint64_t audit_read_last_sig() {
    FILE *f = fopen(AUDIT_SIG, "rb");
    if (!f) return 0;
    uint64_t sig = 0;
    fread(&sig, sizeof(sig), 1, f);
    fclose(f);
    return sig;
}

static int audit_write_last_sig(uint64_t sig) {
    FILE *f = fopen(AUDIT_SIG, "wb");
    if (!f) return -1;
    fwrite(&sig, sizeof(sig), 1, f);
    fclose(f);
    return 0;
}

/* Get current time string */
static void now_str(char *buf, size_t sz) {
    time_t t = time(NULL);
    struct tm lt;
    localtime_r(&t, &lt);
    strftime(buf, sz, "%Y-%m-%d %H:%M:%S%z", &lt);
}

/* --- ACL functions --- */
int acl_set(const char *path, const char *user, const char *perms, int recursive, int is_default) {
    ensure_security_dirs();
    char safe[512]; path_to_safe(path, safe, sizeof(safe));
    char filename[1024];
    if (is_default) snprintf(filename, sizeof(filename), "%s/%s.default.acl", ACL_DIR, safe);
    else snprintf(filename, sizeof(filename), "%s/%s.acl", ACL_DIR, safe);

    // Add or replace the user:perms entry
    // Read existing lines
    FILE *f = fopen(filename, "a+");
    if (!f) { perror("acl_set fopen"); return -1; }

    // Read into memory, and remove existing user entry if present
    fseek(f, 0, SEEK_SET);
    char *buf = NULL;
    size_t cap = 0;
    ssize_t linelen;
    char line[512];
    // We'll write to temp file instead for simpler logic
    char tmpname[1024];
    snprintf(tmpname, sizeof(tmpname), "%s/.tmp_%s", ACL_DIR, safe);
    FILE *tmp = fopen(tmpname, "w");
    if (!tmp) { fclose(f); perror("tmp fopen"); return -1; }

    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char existing_user[128], existing_perms[128];
        if (sscanf(line, "%127[^:]:%127s", existing_user, existing_perms) == 2) {
            if (strcmp(existing_user, user) == 0) {
                // replace
                fprintf(tmp, "%s:%s\n", user, perms);
                found = 1;
            } else {
                fputs(line, tmp);
            }
        } else {
            fputs(line, tmp);
        }
    }
    if (!found) {
        fprintf(tmp, "%s:%s\n", user, perms);
    }
    fclose(f);
    fclose(tmp);
    // rename
    rename(tmpname, filename);

    // Audit
    char details[1024];
    snprintf(details, sizeof(details), "ACL set %s -> %s:%s (default=%d)", path, user, perms, is_default);
    char *u = getenv("USER"); if (!u) u = "unknown";
    audit_log("ACL_SET", u, details);

    // Recursive: apply to child entries (simple directory traversal)
    if (recursive) {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            // iterate dir
            DIR *d = opendir(path);
            if (d) {
                struct dirent *ent;
                while ((ent = readdir(d))) {
                    if (strcmp(ent->d_name, ".")==0 || strcmp(ent->d_name,"..")==0) continue;
                    char child[1024];
                    snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
                    acl_set(child, user, perms, recursive, is_default);
                }
                closedir(d);
            }
        }
    }

    return 0;
}

int acl_get(const char *path, acl_entry_t *entries, int *count) {
    ensure_security_dirs();
    char safe[512]; path_to_safe(path, safe, sizeof(safe));
    char filename[1024]; snprintf(filename, sizeof(filename), "%s/%s.acl", ACL_DIR, safe);
    FILE *f = fopen(filename, "r");
    int c = 0;
    if (!f) {
        *count = 0;
        return 0;
    }
    char line[512];
    while (fgets(line, sizeof(line), f) && c < *count) {
        char user[128], perms[128];
        if (sscanf(line, "%127[^:]:%127s", user, perms) == 2) {
            strncpy(entries[c].user, user, sizeof(entries[c].user)-1);
            strncpy(entries[c].permissions, perms, sizeof(entries[c].permissions)-1);
            entries[c].user[sizeof(entries[c].user)-1]=0;
            entries[c].permissions[sizeof(entries[c].permissions)-1]=0;
            c++;
        }
    }
    fclose(f);
    *count = c;
    return 0;
}

int acl_remove(const char *path, const char *user, int recursive) {
    ensure_security_dirs();
    char safe[512]; path_to_safe(path, safe, sizeof(safe));
    char filename[1024]; snprintf(filename, sizeof(filename), "%s/%s.acl", ACL_DIR, safe);

    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char tmpname[1024]; snprintf(tmpname, sizeof(tmpname), "%s/.tmp_remove_%s", ACL_DIR, safe);
    FILE *tmp = fopen(tmpname, "w");
    if (!tmp) { fclose(f); return -1; }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char existing_user[128], perms[128];
        if (sscanf(line, "%127[^:]:%127s", existing_user, perms) == 2) {
            if (strcmp(existing_user, user) == 0) {
                continue; // skip (remove)
            } else {
                fputs(line, tmp);
            }
        } else {
            fputs(line, tmp);
        }
    }
    fclose(f); fclose(tmp);
    rename(tmpname, filename);

    char details[512];
    snprintf(details, sizeof(details), "ACL remove %s -> %s", path, user);
    char *u = getenv("USER"); if (!u) u = "unknown";
    audit_log("ACL_REMOVE", u, details);

    if (recursive) {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            DIR *d = opendir(path);
            if (d) {
                struct dirent *ent;
                while ((ent = readdir(d))) {
                    if (strcmp(ent->d_name, ".")==0 || strcmp(ent->d_name,"..")==0) continue;
                    char child[1024];
                    snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
                    acl_remove(child, user, recursive);
                }
                closedir(d);
            }
        }
    }

    return 0;
}

/* --- attributes (immutable / append-only) --- */
int attr_set(const char *path, const char *flags) {
    ensure_security_dirs();
    char safe[512]; path_to_safe(path, safe, sizeof(safe));
    char fname[1024]; snprintf(fname, sizeof(fname), "%s/%s.attr", ATTR_DIR, safe);

    // read existing flags
    char existing[256] = "";
    FILE *f = fopen(fname, "r");
    if (f) { fgets(existing, sizeof(existing), f); fclose(f); }

    // parse flags like "+i" or "-i" etc; combine with existing
    char out[256];
    strncpy(out, existing, sizeof(out)-1); out[sizeof(out)-1]=0;

    // simple approach: if flags contains "+i" add "i"; if "-i" remove "i", same for "a"
    if (strstr(flags, "+i")) {
        if (!strchr(out,'i')) {
            strncat(out, "i", sizeof(out)-strlen(out)-1);
        }
    }
    if (strstr(flags, "-i")) {
        char tmp[256]; int p=0;
        for (int i=0; out[i]; ++i) if (out[i] != 'i') tmp[p++]=out[i];
        tmp[p]=0; strcpy(out,tmp);
    }
    if (strstr(flags, "+a")) {
        if (!strchr(out,'a')) strncat(out, "a", sizeof(out)-strlen(out)-1);
    }
    if (strstr(flags, "-a")) {
        char tmp[256]; int p=0;
        for (int i=0; out[i]; ++i) if (out[i] != 'a') tmp[p++]=out[i];
        tmp[p]=0; strcpy(out,tmp);
    }

    FILE *w = fopen(fname, "w");
    if (!w) return -1;
    fprintf(w, "%s\n", out);
    fclose(w);

    char details[512]; snprintf(details, sizeof(details), "ATTR_SET %s -> %s", path, out);
    char *u = getenv("USER"); if (!u) u = "unknown";
    audit_log("ATTR_SET", u, details);

    return 0;
}

int attr_get(const char *path, char *outflags, int outsz) {
    ensure_security_dirs();
    char safe[512]; path_to_safe(path, safe, sizeof(safe));
    char fname[1024]; snprintf(fname, sizeof(fname), "%s/%s.attr", ATTR_DIR, safe);
    FILE *f = fopen(fname, "r");
    if (!f) { if (outflags && outsz>0) outflags[0]=0; return -1; }
    fgets(outflags, outsz, f);
    // trim newline
    size_t L = strlen(outflags); if (L && outflags[L-1]=='\n') outflags[L-1]=0;
    fclose(f);
    return 0;
}

/* --- simulated LUKS --- */
/* We will store a meta file:
   LUKS_DIR/<name>.meta with:
     device=/dev/loop3
     name=secure_vol
     is_open=0
     dm_name=secure_vol
     pass=<passphrase>   <-- WARNING: plain text in simulation
*/

int luks_format(const char *device, const char *name, const char *passphrase) {
    ensure_security_dirs();
    char fname[512]; snprintf(fname, sizeof(fname), "%s/%s.meta", LUKS_DIR, name);
    // do not overwrite existing
    if (access(fname, F_OK) == 0) { fprintf(stderr, "luks_format: name exists\n"); return -1; }

    FILE *f = fopen(fname, "w");
    if (!f) { perror("luks_format"); return -1; }
    fprintf(f, "device=%s\nname=%s\nis_open=0\ndm_name=%s\npass=%s\n", device, name, name, passphrase);
    fclose(f);

    char details[512]; snprintf(details, sizeof(details), "LUKS_FORMAT name=%s device=%s", name, device);
    char *u = getenv("USER"); if (!u) u = "unknown";
    audit_log("LUKS_FORMAT", u, details);

    return 0;
}

static int read_meta_field(const char *metafile, const char *key, char *out, size_t outsz) {
    FILE *f = fopen(metafile, "r");
    if (!f) return -1;
    char line[512];
    int ret = -1;
    while (fgets(line, sizeof(line), f)) {
        char k[128], v[384];
        if (sscanf(line, "%127[^=]=%383[^\n]", k, v) == 2) {
            if (strcmp(k, key) == 0) {
                strncpy(out, v, outsz-1); out[outsz-1]=0; ret = 0; break;
            }
        }
    }
    fclose(f);
    return ret;
}

int luks_open(const char *device_or_name, const char *name, const char *passphrase) {
    ensure_security_dirs();
    // The user may call with either (device + name) or (name only)
    char meta1[512]; snprintf(meta1, sizeof(meta1), "%s/%s.meta", LUKS_DIR, name);
    if (access(meta1, F_OK) != 0) {
        fprintf(stderr, "luks_open: volume %s not found\n", name);
        return -1;
    }
    char stored_pass[256];
    if (read_meta_field(meta1, "pass", stored_pass, sizeof(stored_pass)) != 0) {
        fprintf(stderr, "luks_open: read meta fail\n"); return -1;
    }
    if (strcmp(stored_pass, passphrase) != 0) {
        fprintf(stderr, "luks_open: wrong passphrase\n"); return -2;
    }
    // mark open
    // rewrite meta with is_open=1
    char device[256]; read_meta_field(meta1, "device", device, sizeof(device));
    FILE *f = fopen(meta1, "w");
    if (!f) return -1;
    fprintf(f, "device=%s\nname=%s\nis_open=1\ndm_name=%s\npass=%s\n", device, name, name, stored_pass);
    fclose(f);

    // create mapper file
    char mapper[512]; snprintf(mapper, sizeof(mapper), "%s/%s", MAPPER_DIR, name);
    int fd = open(mapper, O_CREAT|O_RDWR, 0644);
    if (fd >= 0) close(fd);

    char details[512]; snprintf(details, sizeof(details), "LUKS_OPEN name=%s device=%s", name, device);
    char *u = getenv("USER"); if (!u) u = "unknown";
    audit_log("LUKS_OPEN", u, details);
    return 0;
}

int luks_close(const char *name) {
    ensure_security_dirs();
    char meta1[512]; snprintf(meta1, sizeof(meta1), "%s/%s.meta", LUKS_DIR, name);
    if (access(meta1, F_OK) != 0) {
        fprintf(stderr, "luks_close: volume %s not found\n", name);
        return -1;
    }
    char device[256], pass[256];
    read_meta_field(meta1, "device", device, sizeof(device));
    read_meta_field(meta1, "pass", pass, sizeof(pass));
    // rewrite with is_open=0
    FILE *f = fopen(meta1, "w");
    if (!f) return -1;
    fprintf(f, "device=%s\nname=%s\nis_open=0\ndm_name=%s\npass=%s\n", device, name, name, pass);
    fclose(f);

    // remove mapper file
    char mapper[512]; snprintf(mapper, sizeof(mapper), "%s/%s", MAPPER_DIR, name);
    unlink(mapper);

    char details[512]; snprintf(details, sizeof(details), "LUKS_CLOSE name=%s", name);
    char *u = getenv("USER"); if (!u) u = "unknown";
    audit_log("LUKS_CLOSE", u, details);

    return 0;
}

/* --- Audit logging --- */
int audit_log(const char *operation, const char *user, const char *details) {
    ensure_security_dirs();
    // read last sig
    uint64_t last_sig = audit_read_last_sig();
    // compose entry
    char ts[64]; now_str(ts, sizeof(ts));
    char entry[2048];
    snprintf(entry, sizeof(entry), "%s|%s|%s|%s\n", ts, user ? user : "unknown", operation, details ? details : "");

    // compute new sig = FNV(last_sig || entry)
    uint64_t combined_hash;
    // first hash last_sig bytes then entry
    uint64_t tmp = last_sig;
    combined_hash = fnv1a_hash64(&tmp, sizeof(tmp));
    uint64_t entry_hash = fnv1a_hash64(entry, strlen(entry));
    // xor combine (simple)
    uint64_t new_sig = combined_hash ^ entry_hash;

    // append entry to log
    FILE *f = fopen(AUDIT_FILE, "a");
    if (!f) return -1;
    fputs(entry, f);
    fclose(f);

    audit_write_last_sig(new_sig);

    return 0;
}

int audit_show(int last_n) {
    ensure_security_dirs();
    FILE *f = fopen(AUDIT_FILE, "r");
    if (!f) { printf("No audit log\n"); return 0; }
    // read all lines into memory
    char **lines = NULL;
    size_t lines_cap = 0;
    size_t lines_len = 0;
    char buf[2048];
    while (fgets(buf, sizeof(buf), f)) {
        if (lines_len+1 > lines_cap) {
            lines_cap = lines_cap ? lines_cap*2 : 64;
            lines = realloc(lines, sizeof(char*)*lines_cap);
        }
        lines[lines_len] = strdup(buf);
        lines_len++;
    }
    fclose(f);
    size_t start = 0;
    if (last_n > 0 && (size_t)last_n < lines_len) start = lines_len - last_n;
    for (size_t i=start;i<lines_len;i++) {
        printf("%s", lines[i]);
        free(lines[i]);
    }
    free(lines);
    return 0;
}

/* Initialization helper */
int security_init() {
    ensure_security_dirs();
    return 0;
}
