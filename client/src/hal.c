#include "hal.h"


/* Patched function for newer PLL not yet supported by opencm3 */
void _rcc_set_main_pll(uint32_t source, uint32_t pllm, uint32_t plln, uint32_t pllp,
                       uint32_t pllq, uint32_t pllr)
{
	RCC_PLLCFGR = (RCC_PLLCFGR_PLLM(pllm) << RCC_PLLCFGR_PLLM_SHIFT) |
		(plln << RCC_PLLCFGR_PLLN_SHIFT) |
		((pllp & 0x1Fu) << 27u) | /* NEWER PLLP */
		(source << RCC_PLLCFGR_PLLSRC_SHIFT) |
		(pllq << RCC_PLLCFGR_PLLQ_SHIFT) |
		(pllr << RCC_PLLCFGR_PLLR_SHIFT) | RCC_PLLCFGR_PLLREN;
}


#define _RCC_CAT(A, B) A ## _ ## B
#define RCC_ID(NAME) _RCC_CAT(RCC, NAME)

__attribute__((unused))
static uint32_t _clock_freq;

static void clock_setup()
{
  rcc_periph_clock_enable(RCC_PWR);
  rcc_periph_clock_enable(RCC_SYSCFG);
  pwr_set_vos_scale(PWR_SCALE1);
  /* The L4R5ZI chip also needs the R1MODE bit in PWR_CR5 register set, but
     OpenCM3 doesn't support this yet. But luckily the default value for the bit
     is 1. */
  /* Benchmark straight from the HSI16 without prescaling */
  rcc_osc_on(RCC_HSI16);
  rcc_wait_for_osc_ready(RCC_HSI16);
  rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
  rcc_set_ppre1(RCC_CFGR_PPRE1_NODIV);
  rcc_set_ppre2(RCC_CFGR_PPRE2_NODIV);
  rcc_osc_off(RCC_PLL);
  rcc_ahb_frequency = 20000000;
  rcc_apb1_frequency = 20000000;
  rcc_apb2_frequency = 20000000;
  _clock_freq = 20000000;
  while(rcc_is_osc_ready(RCC_PLL));
  /* Configure the PLL oscillator (use CUBEMX tool -> scale HSI16 to 20MHz). */
  _rcc_set_main_pll(RCC_PLLCFGR_PLLSRC_HSI16, 1, 10, 2, RCC_PLLCFGR_PLLQ_DIV2, RCC_PLLCFGR_PLLR_DIV8);
  /* Enable PLL oscillator and wait for it to stabilize. */
  rcc_osc_on(RCC_PLL);
  flash_dcache_enable();
  flash_icache_enable();
  flash_set_ws(FLASH_ACR_LATENCY_0WS);
  flash_prefetch_enable();
  rcc_set_sysclk_source(RCC_CFGR_SW_PLL);
  rcc_wait_for_sysclk_status(RCC_PLL);
}

void usart_setup()
{
  rcc_periph_clock_enable(SERIAL_RCC_GPIO);
  rcc_periph_clock_enable(SERIAL_RCC);
  PWR_CR2 |= PWR_CR2_IOSV;
  gpio_set_output_options(SERIAL_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, SERIAL_PINS);
  gpio_set_af(SERIAL_GPIO, GPIO_AF8, SERIAL_PINS);
  gpio_mode_setup(SERIAL_GPIO, GPIO_MODE_AF, GPIO_PUPD_NONE, SERIAL_PINS);
  usart_set_baudrate(SERIAL_USART, SERIAL_BAUD);
  usart_set_databits(SERIAL_USART, 8);
  usart_set_stopbits(SERIAL_USART, USART_STOPBITS_1);
  usart_set_mode(SERIAL_USART, USART_MODE_TX_RX);
  usart_set_parity(SERIAL_USART, USART_PARITY_NONE);
  usart_set_flow_control(SERIAL_USART, USART_FLOWCONTROL_NONE);
  usart_disable_rx_interrupt(SERIAL_USART);
  usart_disable_tx_interrupt(SERIAL_USART);
  usart_enable(SERIAL_USART);
}

void systick_setup()
{
  /* Systick is always the same on libopencm3 */
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_set_reload(9999999);
  systick_interrupt_enable();
  systick_counter_enable();
}

void rng_setup(void) {
  rcc_osc_on(RCC_HSI48);
  rcc_wait_for_osc_ready(RCC_HSI48);
  rcc_periph_clock_enable(RCC_RNG);
  rcc_set_clock48_source(RCC_CCIPR_CLK48SEL_HSI48);
  rng_enable();
}

void hal_setup()
{
  clock_setup();
  usart_setup();
  rng_setup();
  systick_setup();
}

void hal_send_str(const char* in)
{
  const char* cur = in;
  while (*cur) {
    usart_send_blocking(SERIAL_USART, *cur);
    cur += 1;
  }
  usart_send_blocking(SERIAL_USART, '\n');
}

void send(uint32_t usart, const uint8_t *const data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        usart_send_blocking(usart, data[i]);
        if(data[i]==13U){
          for (uint32_t j = 0; j < 10; j++)
          {
            usart_send_blocking(usart, 14U);
          }
        }
    }
}

void recv(uint32_t usart, uint8_t *const data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        data[i] = usart_recv_blocking(usart);
    }
}

void sys_tick_handler(void)
{
  ++overflowcnt;
}

uint64_t hal_get_time()
{

  while (true) {
    unsigned long long before = overflowcnt;
    unsigned long long result = (before + 1) * (10000000) - systick_get_value();
    if (overflowcnt == before) {
      return result;
    }
  }
}

int randombytes(uint8_t *obuf, size_t len) {
    union {
        unsigned char aschar[4];
        uint32_t asint;
    } random;

    while (len > 4) {
        random.asint = rng_get_random_blocking();
        *obuf++ = random.aschar[0];
        *obuf++ = random.aschar[1];
        *obuf++ = random.aschar[2];
        *obuf++ = random.aschar[3];
        len -= 4;
    }
    if (len > 0) {
        for (random.asint = rng_get_random_blocking(); len > 0; --len) {
            *obuf++ = random.aschar[len - 1];
        }
    }

    return 0;
}
