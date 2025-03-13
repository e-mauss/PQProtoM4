#include "bench.h"
#include "hal.h"


/**
  * @brief  The application entry point.
  * @retval int
*/

bench_t b;
uint32_t fd;

int main(void) {
    fd = SERIAL_USART;
    hal_setup();

    // while(1){}

    bench(1);
    send(fd, (uint8_t *) &b, sizeof b);
}

