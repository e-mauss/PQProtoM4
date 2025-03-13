#include "bench.h"

extern uint32_t fd;
extern bench_t b;
void bench (uint32_t n_samples){
    const int n_ints = 1000;
    uint32_t ints[n_ints];
    for (uint32_t i = 0; i < n_ints; i++){
        ints[i] = i;
    }

    b.send = hal_get_time();
    send(fd, (uint8_t *) ints, sizeof ints);
    b.send = (hal_get_time() - b.send);

    b.recv = hal_get_time();
    recv(fd, (uint8_t *) ints, sizeof ints);
    b.recv = (hal_get_time() - b.recv);
    
    uint8_t correct = 13;
    for (uint32_t i = 0; i < n_ints; i++){
        if(ints[i] != n_ints-i) correct = 0;
    }
    send(fd, &correct, sizeof correct);
}