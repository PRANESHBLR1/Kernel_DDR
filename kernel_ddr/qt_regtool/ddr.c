// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/version.h>

#define DEVICE_NAME "ddr"
#define CLASS_NAME  "ddr_class"

// IOCTL magic + commands
#define DDR_IOC_MAGIC  'k'
#define DDR_READ       _IOWR(DDR_IOC_MAGIC, 1, struct ddr_rw_args)
#define DDR_WRITE      _IOW(DDR_IOC_MAGIC,  2, struct ddr_rw_args)
#define DDR_READ_RANGE _IOWR(DDR_IOC_MAGIC, 3, struct ddr_range_args)
#define DDR_WRITE_RANGE _IOW(DDR_IOC_MAGIC, 4, struct ddr_range_args)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pranesh");
MODULE_DESCRIPTION("DDR register read/write kernel module with word alignment and overwrite protection");
MODULE_VERSION("1.0");

static int ddr_major;
static struct class *ddr_class;
static struct device *ddr_device;

struct ddr_rw_args {
    unsigned long addr;
    u32 value;
};

struct ddr_range_args {
    unsigned long addr;
    u32 values[256];
    int count;
};

static long ddr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct ddr_rw_args rw_args;
    struct ddr_range_args range_args;
    void __iomem *vaddr;
    int i;

    switch (cmd) {
    case DDR_READ:
        if (copy_from_user(&rw_args, (void __user *)arg, sizeof(rw_args)))
            return -EFAULT;

        if (rw_args.addr % 4 != 0)
            return -EINVAL; // must be 32-bit aligned

        vaddr = ioremap(rw_args.addr, 4);
        if (!vaddr)
            return -ENOMEM;

        rw_args.value = ioread32(vaddr);
        iounmap(vaddr);

        if (copy_to_user((void __user *)arg, &rw_args, sizeof(rw_args)))
            return -EFAULT;
        break;

    case DDR_WRITE:
        if (copy_from_user(&rw_args, (void __user *)arg, sizeof(rw_args)))
            return -EFAULT;

        if (rw_args.addr % 4 != 0)
            return -EINVAL;

        vaddr = ioremap(rw_args.addr, 4);
        if (!vaddr)
            return -ENOMEM;

        /* DO NOT OVERWRITE if non-zero: return -EEXIST */
        if (ioread32(vaddr) != 0) {
            iounmap(vaddr);
            return -EEXIST;
        }

        iowrite32(rw_args.value, vaddr);
        iounmap(vaddr);
        break;

    case DDR_READ_RANGE:
        if (copy_from_user(&range_args, (void __user *)arg, sizeof(range_args)))
            return -EFAULT;

        if (range_args.addr % 4 != 0)
            return -EINVAL;

        if (range_args.count <= 0 || range_args.count > 256)
            return -EINVAL;

        for (i = 0; i < range_args.count; i++) {
            unsigned long addr = range_args.addr + i * 4;
            vaddr = ioremap(addr, 4);
            if (!vaddr)
                return -ENOMEM;

            range_args.values[i] = ioread32(vaddr);
            iounmap(vaddr);
        }

        if (copy_to_user((void __user *)arg, &range_args, sizeof(range_args)))
            return -EFAULT;
        break;

    case DDR_WRITE_RANGE:
        if (copy_from_user(&range_args, (void __user *)arg, sizeof(range_args)))
            return -EFAULT;

        if (range_args.addr % 4 != 0)
            return -EINVAL;

        if (range_args.count <= 0 || range_args.count > 256)
            return -EINVAL;

        /* First pass: if ANY target is non-zero, abort and return -EEXIST */
        for (i = 0; i < range_args.count; i++) {
            unsigned long addr = range_args.addr + i * 4;
            vaddr = ioremap(addr, 4);
            if (!vaddr)
                return -ENOMEM;

            if (ioread32(vaddr) != 0) {
                iounmap(vaddr);
                return -EEXIST;
            }
            iounmap(vaddr);
        }

        /* Second pass: all clear, perform writes */
        for (i = 0; i < range_args.count; i++) {
            unsigned long addr = range_args.addr + i * 4;
            vaddr = ioremap(addr, 4);
            if (!vaddr)
                return -ENOMEM;

            iowrite32(range_args.values[i], vaddr);
            iounmap(vaddr);
        }
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = ddr_ioctl,
};

static int __init ddr_init(void)
{
    ddr_major = register_chrdev(0, DEVICE_NAME, &fops);
    if (ddr_major < 0) {
        pr_err("Failed to register char device\n");
        return ddr_major;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    ddr_class = class_create(CLASS_NAME);
#else
    ddr_class = class_create(THIS_MODULE, CLASS_NAME);
#endif
    if (IS_ERR(ddr_class)) {
        unregister_chrdev(ddr_major, DEVICE_NAME);
        pr_err("Failed to register device class\n");
        return PTR_ERR(ddr_class);
    }

    ddr_device = device_create(ddr_class, NULL, MKDEV(ddr_major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ddr_device)) {
        class_destroy(ddr_class);
        unregister_chrdev(ddr_major, DEVICE_NAME);
        pr_err("Failed to create device\n");
        return PTR_ERR(ddr_device);
    }

    pr_info("DDR module loaded successfully\n");
    return 0;
}

static void __exit ddr_exit(void)
{
    device_destroy(ddr_class, MKDEV(ddr_major, 0));
    class_destroy(ddr_class);
    unregister_chrdev(ddr_major, DEVICE_NAME);
    pr_info("DDR module unloaded\n");
}

module_init(ddr_init);
module_exit(ddr_exit);
