#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


int initUsart(const char *filename);

int recv(int fd, void *buf, size_t nbytes);

int send(int fd, void *buf, size_t nbytes);
