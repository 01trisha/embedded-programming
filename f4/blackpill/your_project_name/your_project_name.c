#include "your_project_name.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#define LED_PORT GPIOC
#define LED_PIN GPIO13
#define LED_RCC RCC_GPIOC

static void clock_setup(void) {
  rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
  rcc_periph_clock_enable(LED_RCC);
}

static void gpio_setup(void) {
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
  gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN);
}

static void delay_ms(uint32_t ms) {
  for (uint32_t i = 0; i < ms * 1000; i++) {
    __asm__("nop");
  }
}

int main(void) {
  clock_setup();
  gpio_setup();

  while (1) {
    gpio_toggle(LED_PORT, LED_PIN);
    delay_ms(100000);
  }
}
