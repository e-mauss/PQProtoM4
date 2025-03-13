#include "bench.h"


extern int fd;

void bench(uint32_t n_samples){
    unsigned char pw[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned char sk[64];

    uint32_t ints[25];
    recv(fd, ints, sizeof ints);

    for (uint32_t i = 0; i < n_samples; i++) {
        ocake_pake_sessionkey(sk, sizeof sk, pw, sizeof pw);
    }

    bench_t b;
    recv(fd, &b, sizeof b);
    fprintf(stdout, "PROTO: %ld\n", b.proto);
}