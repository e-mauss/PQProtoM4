#include "bench.h"


extern bench_t b;

void bench (uint32_t n_samples){
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
    unsigned char ss1[CRYPTO_BYTES];
    unsigned char ss2[CRYPTO_BYTES];

    b.keypair = hal_get_time();
    for (uint32_t i = 0; i < n_samples; i++) {
        crypto_kem_keypair(pk, sk);
    }
    b.keypair = (hal_get_time() - b.keypair)/n_samples;

    b.encap = hal_get_time();
    for (uint32_t i = 0; i < n_samples; i++) {
        crypto_kem_enc(ct, ss1, pk);
    }
    b.encap = (hal_get_time() - b.encap)/n_samples;

    b.decap = hal_get_time();
    for (uint32_t i = 0; i < n_samples; i++) {
        crypto_kem_dec(ss2, ct, sk);
    }
    b.decap = (hal_get_time() - b.decap)/n_samples;
}