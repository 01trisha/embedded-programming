#include "your_project_name.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

// Убедитесь, что эти определения соответствуют вашей плате
#define LED_PORT GPIOC
#define LED_PIN GPIO13
#define LED_RCC RCC_GPIOC

static void clock_setup(void) {
  // Настройка тактирования от внешнего кварца 25 МГц на 96 МГц
  rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
  rcc_periph_clock_enable(LED_RCC); // Включаем тактирование порта светодиода
}

static void gpio_setup(void) {
  // Настраиваем пин светодиода как выход
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
  gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN);
}

// Простая функция задержки
static void delay_ms(uint32_t ms) {
  for (uint32_t i = 0; i < ms * 1000; i++) {
    __asm__("nop");
  }
}

int main(void) {
  clock_setup();
  gpio_setup();

  while (1) {
    gpio_toggle(LED_PORT, LED_PIN); // Переключаем состояние светодиода
    delay_ms(300000);               // Задержка 3 секунды
  }
}
