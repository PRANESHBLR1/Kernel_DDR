#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

static unsigned int *buffer;

static int __init diag_init(void) {
    buffer = kmalloc(sizeof(unsigned int), GFP_KERNEL);
    if (!buffer) {
        pr_err("diag_reader: Failed to allocate memory\n");
        return -ENOMEM;
    }

    *buffer = 0xDEADBEEF;  // Simulate a "read" value

    pr_info("diag_reader: Simulated value = 0x%X\n", *buffer);
    return 0;
}

static void __exit diag_exit(void) {
    kfree(buffer);
    pr_info("diag_reader: module unloaded\n");
}

module_init(diag_init);
module_exit(diag_exit);
MODULE_LICENSE("GPL");

