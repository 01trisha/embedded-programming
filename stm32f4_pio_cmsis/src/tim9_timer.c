#include "stm32f4xx.h"

void setup(void) {
    RCC->AHB1ENR |= (1 << 2);

    GPIOC->MODER &= ~(0x03UL << 26);
    GPIOC->MODER |=  (0x01UL << 26);
    GPIOC->OTYPER &= ~(1UL << 13);
    GPIOC->OSPEEDR |=  (0x03UL << 26);
    GPIOC->PUPDR &= ~(0x03UL << 26);

    RCC->APB2ENR |= (1 << 16);
    TIM9->PSC = 15999;//16 МГц/16000 = 1 кГц (1мс) 
    TIM9->ARR = 999;//1сек
    TIM9->DIER |= (1 << 0);
    TIM9->CR1 |= (1 << 0);
}

void TIM1_BRK_TIM9_IRQHandler(void) {
    if (TIM9->SR & (1 << 0)) {//установлен ли uif
        TIM9->SR &= ~(1 << 0);//сброс
        GPIOC->ODR ^= (1 << 13);//свитч состояния
    }
}

int main(void) {
    setup();
    
    NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0);
    
    while (1) {
    }
}