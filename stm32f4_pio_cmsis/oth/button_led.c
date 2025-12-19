#include "stm32f4xx.h"
#include <stdint.h>


void setup(){
    RCC->AHB1ENR |= (1 << 0);
    RCC->AHB1ENR |= (1 << 2);
}

int main(void){
    setup();

    GPIOA->MODER &= ~(0x03 << 0);
    GPIOA->PUPDR &= ~(0x03 << 0);
    GPIOA->PUPDR |= (0x01 << 0);

    GPIOC->MODER &= ~(0x03 << 26);
    GPIOC->MODER |= (0x01 << 26);

    while(1){
        uint32_t check = GPIOA->IDR & (1 << 0); 
        if (!check){
            GPIOC->ODR &= ~(1 << 13);
        }else{
            GPIOC->ODR |= (1 << 13);
        }
    }
}