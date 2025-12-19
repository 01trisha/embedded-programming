#include "stm32f4xx.h"

volatile uint8_t debounce_active = 0;//активен ли дребезг
volatile uint8_t toggle_led = 0;
volatile uint8_t led_state = 0;

void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1 << 0)) {
        //сбрасываем флаг прерывания
        EXTI->PR |= (1 << 0);

        //от повторного запуска навсякий
        if (debounce_active) return;

        debounce_active = 1;

        TIM9->PSC = 15999;// 16 МГц/16000 = 1 кГц (1 мс)
        TIM9->ARR = 19;// 20 тактов (20 мс)
        TIM9->EGR |= (1 << 0);//сброс счетчика
        TIM9->SR &= ~(1 << 0);//сброс uif
        TIM9->CR1 |= (1 << 0);
    }
}

void TIM1_BRK_TIM9_IRQHandler(void) {
    if (TIM9->SR & (1 << 0)) {//устанвлен ли uif
        TIM9->SR &= ~(1 << 0);//сбрасываем его
        TIM9->CR1 &= ~(1 << 0);

        if ((GPIOA->IDR & (1 << 0)) == 0) {
            toggle_led = 1;
        }

        debounce_active = 0;
    }
}

int main(void) {
    RCC->AHB1ENR |= (1 << 0);
    RCC->AHB1ENR |= (1 << 2);
    RCC->APB2ENR |= (1 << 14);//syscfg
    RCC->APB2ENR |= (1 << 16);//tim9

    GPIOA->MODER &= ~(0x03 << 0);
    GPIOA->PUPDR &= ~(0x03 << 0);
    GPIOA->PUPDR |=  (0x01 << 0);

    GPIOC->MODER &= ~(0x03 << 26);
    GPIOC->MODER |=  (0x01 << 26);
    GPIOC->OTYPER &= ~(1 << 13);
    GPIOC->OSPEEDR |= (0x03 << 26);
    GPIOC->PUPDR &= ~(0x03 << 26);

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0_Msk;
    SYSCFG->EXTICR[0] |= 0x0000;//a0 -> exti0 

    EXTI->FTSR |= (1 << 0);//прерывание по нажатию(спаду)
    EXTI->IMR |= (1 << 0);//разр прерывания
    EXTI->PR |= (1 << 0);//cброс флага            // Сброс флага

    TIM9->DIER |= (1 << 0);//прерывание по uif
    TIM9->CR1 &= ~(1 << 0);

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_SetPriority(EXTI0_IRQn, 2);

    NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 1);

    while (1) {
        if (toggle_led) {
            led_state = !led_state;
            GPIOC->ODR = (GPIOC->ODR & ~(1 << 13)) | (led_state << 13);
            toggle_led = 0;
        }
    }
}