#include "bench.h"


extern int fd;

void bench(uint32_t n_samples){
    unsigned char pw[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned char sk[64];

    for (uint32_t i = 0; i < n_samples; i++) {
        ocake_pake_sessionkey(sk, sizeof sk, pw, sizeof pw);
    }

    bench_t b;
    recv(fd, &b, sizeof b);
    fprintf(stdout, "PROTO: %ld\n", b.proto);
    fprintf(stdout, "KEY_GEN: %ld\n", b.key_gen);
    fprintf(stdout, "DECAP: %ld\n", b.decap);
    fprintf(stdout, "AES: %ld\n", b.aes);
    fprintf(stdout, "SHA: %ld\n", b.sha);
    fprintf(stdout, "SHAKE: %ld\n", b.shake);
    fprintf(stdout, "OTHER: %ld\n", b.other);
    fprintf(stdout, "COMM: %ld\n", b.comm);
    uint64_t total = b.shake+b.sha+b.aes+b.decap+b.key_gen;
    fprintf(stdout, "TOTAL: %ld\n", total);
    fprintf(stdout, "TOTAL+COMM+Other: %ld\n", total+b.comm+b.other);
}