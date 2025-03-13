#include "bench.h"


extern uint32_t fd;
extern bench_t b;
size_t spraysize = 50000;
uint32_t pattern_block = 0xDEADBEEF;

void spray(uint32_t **p) {
    uint32_t pattern[spraysize];
    for (size_t i = 0; i < spraysize; ++i) {
        pattern[i] = pattern_block;
    }
    send(fd, (uint8_t *) pattern, 100);
    *p = pattern;
}

void bench(uint32_t n_trials) {
    uint32_t *p_pattern;
    spray(&p_pattern);

    ocake_wrapper();

    for (size_t i = 0; i < 50000; ++i) {
        if(*(&p_pattern-i)==0xDEADBEEF){
            if(*(&p_pattern-i-1)==0xDEADBEEF){
                if(*(&p_pattern-i-2)==0xDEADBEEF) {
                    b.proto = (i-2)*sizeof(unsigned int);
                    break;
                }
            }
        }
    }
}

void ocake_wrapper(){
    uint8_t pw[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t sk[64];
    ocake_pake_sessionkey(sk, sizeof sk, pw, sizeof pw);
}