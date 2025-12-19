#include "stm32f4xx.h"
#include <string.h>

char buffer[16];
int pos = 0;

void uart_init(void) {
    RCC->AHB1ENR |= (1 << 0);
    RCC->APB1ENR |= (1 << 17);//для uart'a

    //a2 tx, a3 rx
    GPIOA->MODER &= ~((0x03 << 4) | (0x03 << 6));
    GPIOA->MODER |=  ((0x02 << 4) | (0x02 << 6));
    
    GPIOA->AFR[0] |= ((7 << 8) | (7 << 12));

    USART2->BRR = 139;// (f_CK / (16 × baudrate)) при 16мгц 

    USART2->CR1 = (1 << 3) | (1 << 2) | (1 << 13);
}

void uart_putc(char c) {
    while (!(USART2->SR & (1 << 7)));
    USART2->DR = c;
}

void handle_command(void) {
    buffer[pos] = 0;

    if (strcmp(buffer, "1") == 0) {
        GPIOC->ODR &= ~(1 << 13);
    }
    else if (strcmp(buffer, "0") == 0) {
        GPIOC->ODR |= (1 << 13);
    }
}

int main(void) {
    RCC->AHB1ENR |= (1 << 2);

    GPIOC->MODER &= ~(0x03 << 26);
    GPIOC->MODER |=  (0x01 << 26);
    GPIOC->OTYPER &= ~(1 << 13);
    GPIOC->OSPEEDR |= (0x03 << 26);

    uart_init();

    while (1) {
        //пришел ли символ
        if (USART2->SR & (1 << 5)) {
            char c = USART2->DR;
            
            if (c == '\r' || c == '\n') {
                if (pos > 0) { //только если есть данные
                    handle_command();
                    pos = 0;
                }
            }
            else if (pos < 15) {
                buffer[pos++] = c;
            }
        }
    }
}