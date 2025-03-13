#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>
#include "api.h"
#include "hal.h"


typedef struct {
    uint64_t keypair;
    uint64_t encap;
    uint64_t decap;
} bench_t;

void bench(uint32_t n_samples);

#endif