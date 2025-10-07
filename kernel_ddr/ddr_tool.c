// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_PATH "/dev/ddr"

// IOCTL definitions (must match ddr.c)
#define DDR_IOC_MAGIC  'k'
#define DDR_READ       _IOWR(DDR_IOC_MAGIC, 1, struct ddr_rw_args)
#define DDR_WRITE      _IOW(DDR_IOC_MAGIC,  2, struct ddr_rw_args)
#define DDR_READ_RANGE _IOWR(DDR_IOC_MAGIC, 3, struct ddr_range_args)
#define DDR_WRITE_RANGE _IOW(DDR_IOC_MAGIC, 4, struct ddr_range_args)

struct ddr_rw_args {
    unsigned long addr;
    unsigned int value;
};

struct ddr_range_args {
    unsigned long addr;
    unsigned int values[256];
    int count;
};

static void usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s read <addr>\n", prog);
    printf("  %s write <addr> <value>\n", prog);
    printf("  %s read_range <addr> <count>\n", prog);
    printf("  %s write_range <addr> <v1> <v2> ...\n", prog);
    exit(1);
}

// Check for 32-bit alignment
static int check_alignment(unsigned long addr)
{
    if (addr % 4 != 0) {
        fprintf(stderr, "Error: Address 0x%lx is not 32-bit aligned\n", addr);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    struct ddr_rw_args rw_args;
    struct ddr_range_args range_args;
    int i;

    if (argc < 3)
        usage(argv[0]);

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (strcmp(argv[1], "read") == 0) {
        rw_args.addr = strtoul(argv[2], NULL, 0);
        if (check_alignment(rw_args.addr) < 0) { close(fd); return 1; }

        if (ioctl(fd, DDR_READ, &rw_args) < 0) {
            perror("ioctl DDR_READ");
            close(fd);
            return 1;
        }
        printf("Value at 0x%lx = 0x%x\n", rw_args.addr, rw_args.value);

    } else if (strcmp(argv[1], "write") == 0) {
        if (argc < 4) usage(argv[0]);
        rw_args.addr = strtoul(argv[2], NULL, 0);
        rw_args.value = strtoul(argv[3], NULL, 0);
        if (check_alignment(rw_args.addr) < 0) { close(fd); return 1; }

        if (ioctl(fd, DDR_WRITE, &rw_args) < 0) {
            perror("ioctl DDR_WRITE");
            close(fd);
            return 1;
        }
        printf("Wrote 0x%x to 0x%lx (only if previously 0)\n", rw_args.value, rw_args.addr);

    } else if (strcmp(argv[1], "read_range") == 0) {
        if (argc < 4) usage(argv[0]);
        range_args.addr = strtoul(argv[2], NULL, 0);
        range_args.count = atoi(argv[3]);
        if (range_args.count > 256) range_args.count = 256;
        if (check_alignment(range_args.addr) < 0) { close(fd); return 1; }

        if (ioctl(fd, DDR_READ_RANGE, &range_args) < 0) {
            perror("ioctl DDR_READ_RANGE");
            close(fd);
            return 1;
        }
        printf("Reading %d values from 0x%lx:\n", range_args.count, range_args.addr);
        for (i = 0; i < range_args.count; i++)
            printf("  [0x%lx] = 0x%x\n", range_args.addr + i*4, range_args.values[i]);

    } else if (strcmp(argv[1], "write_range") == 0) {
        if (argc < 4) usage(argv[0]);
        range_args.addr = strtoul(argv[2], NULL, 0);
        range_args.count = argc - 3;
        if (range_args.count > 256) range_args.count = 256;
        if (check_alignment(range_args.addr) < 0) { close(fd); return 1; }

        for (i = 0; i < range_args.count; i++)
            range_args.values[i] = strtoul(argv[3 + i], NULL, 0);

        if (ioctl(fd, DDR_WRITE_RANGE, &range_args) < 0) {
            perror("ioctl DDR_WRITE_RANGE");
            close(fd);
            return 1;
        }
        printf("Wrote %d values to 0x%lx (only if previously 0)\n",
               range_args.count, range_args.addr);

    } else {
        usage(argv[0]);
    }

    close(fd);
    return 0;
}
