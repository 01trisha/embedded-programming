#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>


static void clock_setup(void){
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
    rcc_periph_clock_enable(RCC_GPIOC);
}

int main(){
    clock_setup();
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    while (1) {
        gpio_toggle(GPIOC, GPIO13);
        for(int i = 0; i < 10000000; i++){
          __asm volatile("nop");
        }
       
    }
}