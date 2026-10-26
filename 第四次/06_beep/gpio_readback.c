/* Read-only inspection of GPIO5 DR/GDIR/PSR on this i.MX6ULL board. */
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(void)
{
    int fd = open("/dev/mem", O_RDONLY | O_SYNC);
    volatile uint32_t *gpio;
    if (fd < 0) {
        perror("open /dev/mem");
        return 1;
    }
    gpio = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0x020ac000);
    if (gpio == MAP_FAILED) {
        perror("mmap GPIO5");
        close(fd);
        return 1;
    }
    printf("GPIO5_IO01 DR=%u GDIR=%u PSR=%u (read only)\n",
           (gpio[0] >> 1) & 1, (gpio[1] >> 1) & 1, (gpio[2] >> 1) & 1);
    munmap((void *)gpio, 4096);
    close(fd);
    return 0;
}
