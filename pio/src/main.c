#include "stm32f4xx.h"
#include <stdio.h>

extern void initialise_monitor_handles(void);

void delay_ms(uint32_t ms) {
    // SystemCoreClock по умолчанию 16 MHz для HSI
    // Простой цикл задержки (не точный, но достаточный)
    for (uint32_t i = 0; i < ms * 4000; i++) {
        __NOP();
    }
}

int main(void) {
    #ifdef DEBUG
    initialise_monitor_handles();
    printf("STM32F411 Debug Start\n");
    #endif
    
    // Включаем тактирование порта GPIOC
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    
    // Настраиваем PC13 как выход (MODE = 01)
    GPIOC->MODER &= ~(GPIO_MODER_MODER13);
    GPIOC->MODER |= GPIO_MODER_MODER13_0;
    
    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT13);
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR13);
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR13);
    
    #ifdef DEBUG
    printf("GPIO C13 configured as output\n");
    #endif
    
    uint32_t counter = 0;
    while (1) {
        GPIOC->BSRR = GPIO_BSRR_BS13;
        #ifdef DEBUG
        printf("LED ON [%lu]\n", counter++);
        #endif
        delay_ms(5000);
        
        GPIOC->BSRR = GPIO_BSRR_BR13;
        #ifdef DEBUG
        printf("LED OFF\n");
        #endif
        delay_ms(5000);
    }
}
