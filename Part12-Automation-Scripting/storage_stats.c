#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mathew");
MODULE_DESCRIPTION("Storage Monitoring Kernel Module");

//Simulated statistics variables
static unsigned long read_ops = 0;
static unsigned long write_ops = 0;
static unsigned long read_bytes = 0;
static unsigned long write_bytes = 0;

#define PROC_NAME "storage_stats"

// Function called when a user reads/proc/storage_stats
static ssize_t stats_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char buffer[256];
    int len;

    read_ops += 5;
    write_ops += 3;
    read_bytes += 4096;
    write_bytes += 2048;

    len = snprintf(buffer, sizeof(buffer),
                   "Storage Stats:\n"
                   "Read Ops: %lu\n"
                   "Write Ops: %lu\n"
                   "Read Bytes: %lu\n"
                   "Write Bytes: %lu\n",
                   read_ops, write_ops, read_bytes, write_bytes);

    return simple_read_from_buffer(buf, count, ppos, buffer, len);
}

static const struct proc_ops proc_file_ops = {
    .proc_read = stats_read,
};

static int __init storage_stats_init(void)
{
    proc_create(PROC_NAME, 0444, NULL, &proc_file_ops);
    printk(KERN_INFO "[storage_stats] Module loaded.\n");
    return 0;
}

static void __exit storage_stats_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "[storage_stats] Module unloaded.\n");
}

module_init(storage_stats_init);
module_exit(storage_stats_exit);
