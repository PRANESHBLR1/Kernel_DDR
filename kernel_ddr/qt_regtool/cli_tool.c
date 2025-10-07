#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define DEVICE "/dev/ddr"

#define DDR_IOC_MAGIC  'k'
#define DDR_READ       _IOWR(DDR_IOC_MAGIC, 1, struct ddr_rw_args)
#define DDR_WRITE      _IOW(DDR_IOC_MAGIC,  2, struct ddr_rw_args)

struct ddr_rw_args {
    unsigned long addr;
    uint32_t value;
};

int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct ddr_rw_args req;
    req.addr = 0x100;
    req.value = 0x55;

    if (ioctl(fd, DDR_WRITE, &req) < 0) {
        perror("ioctl write");
    } else {
        printf("Wrote 0x%x to [0x%lx]\n", req.value, req.addr);
    }

    if (ioctl(fd, DDR_READ, &req) < 0) {
        perror("ioctl read");
    } else {
        printf("Read [0x%lx] = 0x%x\n", req.addr, req.value);
    }

    close(fd);
    return 0;
}
