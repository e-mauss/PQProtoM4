#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>
#include "io.h"
#include "ocake.h"


typedef struct {
    uint64_t proto;
} bench_t;


void bench(uint32_t n_samples);

#endif