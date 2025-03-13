#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>
#include "ocake.h"
#include "hal.h"


typedef struct {
    uint64_t proto;
} bench_t;

void spray(uint32_t **p);

void ocake_wrapper();

void bench(uint32_t n_trials);

#endif