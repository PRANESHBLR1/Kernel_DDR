#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/seq_file.h>

#define TARGET_ADDR 0xFE000000
#define MAP_SIZE    0x1000

static void __iomem *mapped;

static int diag_proc_read(struct seq_file *file, void *v) {
    unsigned int value = ioread32(mapped);
    seq_printf(file, "Value at 0x%x = 0x%08x\n", TARGET_ADDR, value);
    return 0;
}

static int diag_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, diag_proc_read, NULL);
}

static const struct proc_ops diag_proc_fops = {
    .proc_open = diag_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init diag_init(void) {
    mapped = ioremap(TARGET_ADDR, MAP_SIZE);
    if (!mapped) {
        pr_err("Failed to map DDR address\n");
        return -ENOMEM;
    }

    proc_create("diag_read", 0, NULL, &diag_proc_fops);
    pr_info("diag_read module loaded\n");
    return 0;
}

static void __exit diag_exit(void) {
    remove_proc_entry("diag_read", NULL);
    iounmap(mapped);
    pr_info("diag_read module unloaded\n");
}

module_init(diag_init);
module_exit(diag_exit);
MODULE_LICENSE("GPL");

