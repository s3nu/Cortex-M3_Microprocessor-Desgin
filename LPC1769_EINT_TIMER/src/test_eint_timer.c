/*
===============================================================================
 Name        : test_eint.c
 Author      : Nicolin Chen
 Mentor      : Dr. Harry Li
 Version     : 1
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdio.h>

#define DEBUG 0

#define BIT(x)				(1UL << (x))
#define MASK(x)				(~(1UL << (x)))
#define GEN_MASK(len, off)	(((1 << len) - 1) << off)

#define pr_debug(fmt, ...) \
	do { \
		if (DEBUG) \
			printf(fmt, ##__VA_ARGS__); \
	} while (0)

#define TIMER_INTERVAL		500
#define PCLKSEL_TIMER0(x)	((x) >> 2 & 0x3)

void TIMER0_IRQHandler (void)
{
	/* clear timer interrupt */
	LPC_TIM0->IR = 0x1;

	pr_debug("---TIMER0 IRQ---\n");

	/* Add your code here */

	return;
}

void EINT0_IRQHandler()
{
	/* Clear EINT0 interrupt */
	LPC_SC->EXTINT = BIT(0);

	/* Toggle the timer switch */
	if (!LPC_TIM0->TCR) {
		pr_debug("enable timer\n");
		LPC_TIM0->TCR = 1;
	} else {
		pr_debug("disable timer\n");
		LPC_TIM0->TCR = 0;
	}
}

int main(void)
{
	/* Power up timer0 */
	LPC_SC->PCONP |= BIT(1);
	/* Clear all interrupts */
	LPC_TIM0->IR = 0x3F;
	/* Set prescaler based on sysclk to get 1000us minimum interval */
	switch (PCLKSEL_TIMER0(LPC_SC->PCLKSEL0)) {
	case 0x00:
		LPC_TIM0->PR = SystemCoreClock / 4000;
		break;
	case 0x01:
		LPC_TIM0->PR = SystemCoreClock / 1000;
		break;
	case 0x02:
		LPC_TIM0->PR = SystemCoreClock / 2000;
		break;
	case 0x03:
	default:
		LPC_TIM0->PR = SystemCoreClock / 8000;
		break;
	}
	/* Set the interval at Match Control 0 */
	LPC_TIM0->MR0 = TIMER_INTERVAL;
	/* Enable Match Control 0 */
	LPC_TIM0->MCR = BIT(0) | BIT(1);
	/* Disable Timer as default */
	LPC_TIM0->TCR = 0;
	/* Enable Timer IRQ */
	NVIC_EnableIRQ(TIMER0_IRQn);

	/* Set P2.10 as EINT0 function */
	LPC_PINCON->PINSEL4 &= ~GEN_MASK(2, 20);
	LPC_PINCON->PINSEL4 |= BIT(20);
	/* Enable falling edge trigger for P2.10 */
	LPC_GPIOINT->IO2IntEnF |= BIT(10);
	/* Edge trigger for EINT0 */
	LPC_SC->EXTMODE |= BIT(0);
	/* Enable EINT IRQ */
	NVIC_EnableIRQ(EINT0_IRQn);

	printf("Main Function: Waiting for interrupt\n");

    while(1);

    return 0;
}
