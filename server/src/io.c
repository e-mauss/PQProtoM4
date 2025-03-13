#include "io.h"


int initUsart(const char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", filename, strerror(errno));
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    tty.c_lflag = 0;                            // no signaling chars, no echo,
    tty.c_oflag = 0;                            // no remapping, no delays
    tty.c_cc[VMIN]  = 1;                        // read blocks
    tty.c_cc[VTIME] = 10;                       // 1 seconds read timeout

    tty.c_iflag &= ~IGNBRK;                     // disable break processing
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // shut off xon/xoff ctrl
    
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_cflag &= ~(PARENB | PARODD);          // shut off parity
    tty.c_cflag |= (CLOCAL | CREAD);            // ignore modem controls, enable reading

    cfsetispeed(&tty, B19200);
    cfsetospeed(&tty, B19200);

    ioctl(fd, TIOCEXCL, NULL);
    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error setting UART config: %s\n", strerror(errno));
        return -1;
    }
    return fd;
}

int recv(int fd, void *buf, size_t nbytes) {
    int received=0;
    uint8_t errbuf[10];
    int correcting;
    uint8_t *ptr = (uint8_t *) buf;
    for(int i = 0; i < nbytes;){
        received=read(fd, ptr+i, 1);
        if(received<=0) return received;

        if(*(ptr+i)==10) {
            int needs_correction = 1;
            for(int j = 0; j < 10;){
                received=read(fd, errbuf+j, 1);
                if(errbuf[j] != 14) {
                    needs_correction = 0;
                } 
                j+=received;
            }
            if (needs_correction){
                *(ptr+i)=13;
            }
            else {
                memcpy(ptr+i+1, errbuf, sizeof errbuf);
                i+=10;
            }
        }

        i += received;
    }

    return nbytes;
}

int send(int fd, void *buf, size_t nbytes) {
    int written=0;
    int to_write=nbytes;
    uint8_t *ptr = (uint8_t *) buf;
    while(ptr < (uint8_t *) buf + nbytes){
        written=write(fd, ptr, to_write);
        
        if(written<0) return written;
        
        ptr+=written;
        to_write-=written;
    }

    return nbytes;
}
