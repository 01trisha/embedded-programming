#include "stm32f4xx.h"
void delay(int time){
    for (volatile uint32_t i = 0; i < time*1000000; i++){

    }
}
int main(){
    RCC->AHB1ENR |= (1 << 2);

    GPIOC->MODER &= ~(0x03 << 26); //очистил
    GPIOC->MODER |= (0x01 << 26);//установил 01 на 26 (27 26)
    //0 на 27 свинуть, 1 сдвинуть на 26 

    while(1){
        GPIOC->ODR ^= (1 << 13);
        delay(1);
    }


}