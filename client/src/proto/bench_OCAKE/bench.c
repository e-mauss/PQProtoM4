#include "bench.h"


extern bench_t b;

void bench(uint32_t n_samples){
    unsigned char pw[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned char sk[64];

    for (uint32_t i = 0; i < n_samples; i++) {
        ocake_pake_sessionkey(sk, sizeof sk, pw, sizeof pw);
    }

    b.proto /= n_samples;
    b.aes /= n_samples;
    b.key_gen /= n_samples;
    b.decap /= n_samples;
    b.sha /= n_samples;
    b.shake /= n_samples;
    b.other /= n_samples;
    b.comm /= n_samples;
}
