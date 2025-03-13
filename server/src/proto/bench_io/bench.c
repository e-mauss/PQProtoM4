#include "bench.h"


extern int fd;

void bench(uint32_t n_samples){
    const int n_ints = 1000;
    uint32_t ints[n_ints];
    recv(fd, ints, sizeof ints);
    for (int i = 0; i < n_ints; i++){
        if(ints[i] != i) fprintf(stderr, "Int at index %d faulty. Is: %d\n", i, ints[i]);
    }
    for (int i = 0; i < n_ints; i++){
        ints[i]=n_ints-i;
    }
    send(fd, ints, sizeof ints);
    uint8_t correct;
    recv(fd, &correct, sizeof correct);
    if (correct){
        printf("Send successful %d\n", correct);
    }
    else {
        printf("Send unsuccessful\n");
    }
}