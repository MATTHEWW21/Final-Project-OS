#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "storage_stats"

static struct proc_dir_entry *proc_entry;

typedef struct {
    unsigned long read_ops;
    unsigned long write_ops;
    unsigned long long read_bytes;
    unsigned long long write_bytes;
} storage_stats_t;

static storage_stats_t stats = {
    .read_ops = 0,
    .write_ops = 0,
    .read_bytes = 0,
    .write_bytes = 0,
};

static int storage_stats_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Read Operations: %lu\n", stats.read_ops);
    seq_printf(m, "Write Operations: %lu\n", stats.write_ops);
    seq_printf(m, "Bytes Read: %llu\n", stats.read_bytes);
    seq_printf(m, "Bytes Written: %llu\n", stats.write_bytes);
    return 0;
}

static int storage_stats_open(struct inode *inode, struct file *file)
{
    return single_open(file, storage_stats_show, NULL);
}

static const struct proc_ops storage_stats_fops = {
    .proc_open    = storage_stats_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init storage_stats_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0444, NULL, &storage_stats_fops);
    if (!proc_entry) {
        pr_err("storage_stats: failed to create /proc entry\n");
        return -ENOMEM;
    }

    pr_info("storage_stats: module loaded\n");
    return 0;
}

static void __exit storage_stats_exit(void)
{
    if (proc_entry)
        proc_remove(proc_entry);

    pr_info("storage_stats: module unloaded\n");
}

module_init(storage_stats_init);
module_exit(storage_stats_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Storage statistics kernel module");
