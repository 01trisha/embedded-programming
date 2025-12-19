#include "stm32f4xx.h"

volatile uint8_t pwm_counter = 0;
volatile uint8_t pwm_duty = 0;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & (1 << 0)) {//uif
        TIM2->SR &= ~(1 << 0);

    if (pwm_counter < pwm_duty) {
        GPIOC->BSRR = (1 << (29));
    } else {
        GPIOC->BSRR = (1 << 13);
    }
        pwm_counter++;
        if (pwm_counter >= 100) {
            pwm_counter = 0;
        }
    }
}

int main(void) {
    RCC->AHB1ENR |= (1 << 2);//gpioc
    RCC->APB1ENR |= (1 << 0);//tim2

    GPIOC->MODER &= ~(0x03UL << 26);
    GPIOC->MODER |=  (0x01UL << 26);
    GPIOC->OTYPER &= ~(1UL << 13);
    GPIOC->OSPEEDR |=  (0x03UL << 26);
    GPIOC->PUPDR &= ~(0x03UL << 26);

    TIM2->PSC = 159; //при 16МГц предделитель 159 чтобы было 16МГц/(159 +1) = 100 000 гц  (100мкс)
    TIM2->ARR = 9; // + 1 будет и счетчик считает до 10 чтобы генерить прерываник 
    TIM2->DIER |= (1 << 0);//разрешаем прерывание
    TIM2->CR1 |= (1 << 0);

    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 0);

    pwm_duty = 100;  //какая яркость (10%)

    while (1) {
    }
}