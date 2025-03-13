#include "bench.h"


extern int fd;

void bench(uint32_t n_samples){
    bench_t b;
    recv(fd, &b, sizeof b);
    fprintf(stdout, "KEYPAIR: %ld\n", b.keypair);
    fprintf(stdout, "ENCAP: %ld\n", b.encap);
    fprintf(stdout, "DECAP: %ld\n", b.decap);
}