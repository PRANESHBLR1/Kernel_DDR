#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#define DEVICE "/dev/ddr"

#define DDR_IOC_MAGIC  'k'
#define DDR_READ        _IOWR(DDR_IOC_MAGIC, 1, struct ddr_rw_args)
#define DDR_WRITE       _IOW(DDR_IOC_MAGIC,  2, struct ddr_rw_args)
#define DDR_READ_RANGE  _IOWR(DDR_IOC_MAGIC, 3, struct ddr_range_args)
#define DDR_WRITE_RANGE _IOW(DDR_IOC_MAGIC,  4, struct ddr_range_args)

struct ddr_rw_args {
    unsigned long addr;
    uint32_t value;
};

struct ddr_range_args {
    unsigned long addr;
    uint32_t values[256];
    int count;
};

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s read <hex_addr>\n", argv[0]);
        fprintf(stderr, "  %s write <hex_addr> <hex_value>\n", argv[0]);
        fprintf(stderr, "  %s readrange <hex_addr> <count>\n", argv[0]);
        fprintf(stderr, "  %s writerange <hex_addr> <val1> <val2> ...\n", argv[0]);
        return 1;
    }

    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (!strcmp(argv[1], "read")) {
        struct ddr_rw_args req = { strtoul(argv[2], NULL, 16), 0 };
        if (ioctl(fd, DDR_READ, &req) < 0) {
            perror("ioctl read");
            return 1;
        }
        printf("Read [0x%lx] = 0x%x\n", req.addr, req.value);

    } else if (!strcmp(argv[1], "write")) {
        if (argc < 4) {
            fprintf(stderr, "Missing value\n");
            return 1;
        }
        struct ddr_rw_args req = { strtoul(argv[2], NULL, 16), strtoul(argv[3], NULL, 16) };
        if (ioctl(fd, DDR_WRITE, &req) < 0) {
            perror("ioctl write");
            return 1;
        }
        printf("Wrote [0x%lx] = 0x%x\n", req.addr, req.value);

    } else if (!strcmp(argv[1], "readrange")) {
        if (argc < 4) {
            fprintf(stderr, "Need address and count\n");
            return 1;
        }
        struct ddr_range_args req;
        req.addr = strtoul(argv[2], NULL, 16);
        req.count = atoi(argv[3]);

        if (ioctl(fd, DDR_READ_RANGE, &req) < 0) {
            perror("ioctl readrange");
            return 1;
        }
        for (int i = 0; i < req.count; i++)
            printf("Read [0x%lx] = 0x%x\n", req.addr + i*4, req.values[i]);

    } else if (!strcmp(argv[1], "writerange")) {
        if (argc < 4) {
            fprintf(stderr, "Need address and values\n");
            return 1;
        }
        struct ddr_range_args req;
        req.addr = strtoul(argv[2], NULL, 16);
        req.count = argc - 3;
        for (int i = 0; i < req.count; i++)
            req.values[i] = strtoul(argv[3+i], NULL, 16);

        if (ioctl(fd, DDR_WRITE_RANGE, &req) < 0) {
            perror("ioctl writerange");
            return 1;
        }
        printf("Wrote %d values starting at 0x%lx\n", req.count, req.addr);

    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
    }

    close(fd);
    return 0;
}
