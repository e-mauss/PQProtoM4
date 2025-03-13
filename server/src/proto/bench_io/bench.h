#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>
#include <stdio.h>
#include "io.h"

typedef struct {
    uint64_t send;
    uint64_t recv;
} bench_t;

void bench(uint32_t n_samples);

#endif