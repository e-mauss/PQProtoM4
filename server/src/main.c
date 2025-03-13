#include "io.h"
#include "bench.h"

int fd;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("No file interface for UART communication specified.\nAbort program...");
        return -1;
    }
    fd = initUsart(argv[1]);

    bench(1);
    return 0;
}
