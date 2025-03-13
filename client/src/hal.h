#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/rng.h>
#include <libopencm3/stm32/pwr.h>

#define SERIAL_BAUD 19200

#define SERIAL_USART LPUART1
#define SERIAL_GPIO GPIOG
#define SERIAL_RCC_GPIO RCC_GPIOG
#define SERIAL_RCC RCC_LPUART1
#define SERIAL_PINS (GPIO8 | GPIO7)
#define SERIAL_AF GPIO_AF8

// #define SERIAL_USART USART2
// #define SERIAL_GPIO GPIOD
// #define SERIAL_RCC_GPIO RCC_GPIOD
// #define SERIAL_RCC RCC_USART2
// #define SERIAL_PINS (GPIO5 | GPIO6)
// #define SERIAL_AF GPIO_AF7

#define NUCLEO_L4R5_BOARD

static volatile unsigned long long overflowcnt = 0;

void hal_setup(void);

void usart_setup(void);

void systick_setup(void);

void rng_setup(void);

void _rcc_set_main_pll(uint32_t source, uint32_t pllm, uint32_t plln, uint32_t pllp,
                       uint32_t pllq, uint32_t pllr);

void hal_send_str(const char* in);

void send(uint32_t usart, const uint8_t * const data, uint32_t len);

void recv(uint32_t usart, uint8_t *data, uint32_t len);

uint64_t hal_get_time(void);

int randombytes(uint8_t *obuf, size_t len);


#endif