#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <physical_address_in_hex>\n", argv[0]);
        return 1;
    }

    off_t addr = strtoul(argv[1], NULL, 16);

    int fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    void *map_base = mmap(NULL, MAP_SIZE, PROT_READ, MAP_SHARED, fd, addr & ~MAP_MASK);
    if (map_base == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    void *virt_addr = map_base + (addr & MAP_MASK);
    unsigned int value = *((volatile unsigned int *) virt_addr);
    printf("Value at physical address 0x%lX: 0x%X\n", (unsigned long)addr, value);

    munmap(map_base, MAP_SIZE);
    close(fd);
    return 0;
}
