#include "delayNS.h"
#include "stm32f10x.h"
#include <stdint.h>

/* Andy Pomfret Delay code - Modified to provide nanoseconds and use timer 2 */
static uint_fast8_t _init = 0;

static void timerSetup(void){
	TIM2->DIER = 0x00000000;    /* Disable TIM2 interrupts */
	TIM2->PSC = 0;				/* 1ns per tick from a 84MHz clock */
	TIM2->EGR = TIM_EGR_UG;		/* Force register update */
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;	/* Enable TIM14 clock */
}

void delay_ns(uint16_t ns){
	ns /= 12;
	if (!_init) timerSetup();
	TIM2->CNT = 0;							/* Reset TIM2 */
	TIM2->CR1 = TIM_CR1_CEN;		/* Start TIM2, source CLK_INT */
	while (TIM2->CNT < ns);
	TIM14->CR1 = 0x00000000;
}

void delay_ms(uint16_t ms){
	uint16_t i;
	if (!_init) timerSetup();
	for(i=0; i < ms; i++){
		delay_us(100);
	}
}

void delay_us(uint16_t us){
		uint16_t i;
	if (!_init) timerSetup();
	for(i=0; i < us; i++){
		delay_ns(1000);
	}
}
