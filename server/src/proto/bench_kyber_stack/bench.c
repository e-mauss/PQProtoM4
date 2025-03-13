#include "bench.h"


extern int fd;

void bench(uint32_t n_samples){
    uint32_t ints[25];
    recv(fd, ints, sizeof ints);
    recv(fd, ints, sizeof ints);
    recv(fd, ints, sizeof ints);
    bench_t b;
    recv(fd, &b, sizeof b);
    fprintf(stdout, "KEYPAIR: %ld\n", b.keypair);
    fprintf(stdout, "ENCAP: %ld\n", b.encap);
    fprintf(stdout, "DECAP: %ld\n", b.decap);
}