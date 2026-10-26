#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    unsigned char command;
    int fd, result = 0;
    if (argc != 3 || (strcmp(argv[2], "0") && strcmp(argv[2], "1"))) {
        fprintf(stderr, "Usage: %s /dev/beep 0|1\n", argv[0]);
        return 2;
    }
    command = argv[2][0] - '0';
    fd = open(argv[1], O_WRONLY);
    if (fd < 0) {
        perror("open beep");
        return 1;
    }
    if (write(fd, &command, 1) != 1) {
        perror("write beep");
        result = 1;
    }
    if (close(fd) < 0) {
        perror("close beep");
        result = 1;
    }
    if (!result)
        printf("beep: %s\n", command ? "ON" : "OFF");
    return result;
}
