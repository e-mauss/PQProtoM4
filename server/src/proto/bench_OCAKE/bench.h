#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>
#include "io.h"
#include "ocake.h"


typedef struct {
    uint64_t proto;
    uint64_t aes;
    uint64_t shake;
    uint64_t sha;
    uint64_t key_gen;
    uint64_t decap;
    uint64_t other;
    uint64_t comm;
} bench_t;

void bench(uint32_t n_samples);

#endif