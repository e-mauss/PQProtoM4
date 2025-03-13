#include "bench.h"


extern uint32_t fd;
extern bench_t b;
size_t spraysize = 100000;
uint32_t pattern_block = 0xDEADBEEF;
unsigned char sk[CRYPTO_SECRETKEYBYTES];
unsigned char pk[CRYPTO_PUBLICKEYBYTES];
unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
unsigned char ss1[CRYPTO_BYTES];
unsigned char ss2[CRYPTO_BYTES];

void spray(uint32_t **p) {
    uint32_t pattern[spraysize];
    for (size_t i = 0; i < spraysize; ++i) {
        pattern[i] = pattern_block;
    }
    send(fd, (uint8_t *) pattern, 100);
    *p = pattern;
}

void bench(uint32_t n_trials) {
    subbench_keypair();
    subbench_encap();
    subbench_decap();
}

void subbench_keypair(void){
    uint32_t *p_pattern;
    spray(&p_pattern);

    crypto_kem_keypair(pk, sk);

    for (size_t i = 0; i < 50000; ++i) {
        if(*(&p_pattern-i)==0xDEADBEEF){
            if(*(&p_pattern-i-1)==0xDEADBEEF){
                if(*(&p_pattern-i-2)==0xDEADBEEF) {
                    b.keypair = (i-2)*sizeof(unsigned int);
                    break;
                }
            }
        }
    }
}

void subbench_encap(void){
    uint32_t *p_pattern;
    spray(&p_pattern);

    crypto_kem_enc(ct, ss1, pk);

    for (size_t i = 0; i < 50000; ++i) {
        if(*(&p_pattern-i)==0xDEADBEEF){
            if(*(&p_pattern-i-1)==0xDEADBEEF){
                if(*(&p_pattern-i-2)==0xDEADBEEF) {
                    b.encap = (i-2)*sizeof(unsigned int);
                    break;
                }
            }
        }
    }
}

void subbench_decap(void){
    uint32_t *p_pattern;
    spray(&p_pattern);

    crypto_kem_dec(ss2, ct, sk);

    for (size_t i = 0; i < 50000; ++i) {
        if(*(&p_pattern-i)==0xDEADBEEF){
            if(*(&p_pattern-i-1)==0xDEADBEEF){
                if(*(&p_pattern-i-2)==0xDEADBEEF) {
                    b.decap = (i-2)*sizeof(unsigned int);
                    break;
                }
            }
        }
    }
}