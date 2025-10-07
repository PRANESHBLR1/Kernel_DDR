#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

#define TARGET_ADDR 0xFE000000  // <- Safe MMIO test address (may vary)
#define MAP_SIZE    0x1000      // 4 KB

static void __iomem *mapped;

static int __init diag_init(void) {
    mapped = ioremap(TARGET_ADDR, MAP_SIZE);
    if (!mapped) {
        pr_err("diag_reader: Failed to map physical address\n");
        return -ENOMEM;
    }

    unsigned int value = ioread32(mapped);
    pr_info("diag_reader: Value at 0x%X = 0x%08X\n", TARGET_ADDR, value);

    return 0;
}

static void __exit diag_exit(void) {
    if (mapped)
        iounmap(mapped);

    pr_info("diag_reader: Module unloaded\n");
}

module_init(diag_init);
module_exit(diag_exit);
MODULE_LICENSE("GPL");

