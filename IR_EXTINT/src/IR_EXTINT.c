/*
 ===============================================================================
 Name        : IR_EXTINT.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "timer.h"
#include "uart.h"
#include "extint.h"
#include "type.h"
#include <time.h>
#endif

#include <cr_section_macros.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// TODO: insert other include files here

// TODO: insert other definitions and declarations here

extern uint32_t timer0_m0_counter, timer1_m0_counter;
extern uint32_t timer0_m1_counter, timer1_m1_counter;

#define LEDPINA		0
#define LEDPINB		4
#define EINTPIN		10

//Iterations before human response
#define COUNTDOWN	8
int timerCount =0 ;
delay = (9000000 / 1000-1);
//Response Time Attributes
int count = 0;
double resultBuffer[4];
double averageTime = 0;
double variance = 0;
int randTimer = 0;
int ready = 0;
int storage[4];

void prepareData(void);
void prepareTimer0(void);

int testing(void);
int testStatus = 0;

//Convert milliseconds to seconds
double convertMstoS(double msValue);

int main(void) {
;	LPC_GPIO2->FIODIR |= (1 << LEDPINA);
	LPC_GPIO2->FIODIR |= (1 << LEDPINB);
	LPC_GPIO2->FIODIR &= ~(1 << EINTPIN);
	LPC_GPIOINT->IO2IntEnF |= (1 << EINTPIN);
	NVIC_EnableIRQ(EINT3_IRQn);
	prepareTimer0();
	printf("Send Initial Signal To BEGIN TEST\n");
	prepareData();
	while (testing())
		;
	printf("\nCompleted\n");
	LPC_GPIO2->FIOCLR |= (1 << LEDPINA);
	LPC_GPIO2->FIOCLR |= (1 << LEDPINB);
	return 0;
}
int testing(void) {
	while (!testStatus) {
	}
	ready = 0;
	LPC_GPIO2->FIOSET |= (1 << LEDPINA);
	randTimer = rand() % 3000 + 2;
	delayMs(0, randTimer);
	printf("randTimer: %d\n", randTimer);
	printf("Press.\n");
	printf("Results: #%d ", (count + 1));
	LPC_GPIO2->FIOSET |= (1 << LEDPINB);
	LPC_TIM0->TCR = 0x02;
	LPC_TIM0->TCR = 0x01;
	while (!ready) {
	}
	if (count >= COUNTDOWN) {
		int n = 0;
		while (n < COUNTDOWN) {
			averageTime += resultBuffer[n];
			n++;
		}
		averageTime = averageTime / COUNTDOWN;
		double averageTimeSec = convertMstoS(averageTime);
		printf("Average Response Time: %.2f sec\n", averageTimeSec);
		n = 0;
		while (n < COUNTDOWN) {
			variance += (resultBuffer[n] - averageTime)
					* (resultBuffer[n] - averageTime);

			n++;
		}
		variance = variance / COUNTDOWN;
		double stdrdDev = sqrt(variance);
		double stdrdDevSec = convertMstoS(stdrdDev);
		printf("STD Dev: %.2f sec\n", stdrdDevSec);
		return 0;
	} else {
		return 1;
	}
}
double convertMstoS(double msValue) {
	double sValue = msValue / 1000;
	return sValue;
}
void prepareTimer0(void) {
	LPC_SC->PCONP |= (1 << 1);
	LPC_TIM0->PR = (SystemCoreClock / 4000) - 1;
	LPC_TIM0->TCR = 0x00;
}
void prepareData(void) {
	count = 0;
	averageTime = 0;
	variance = 0;
	resultBuffer[4] = 0;
}
void EINT3_IRQHandler(void) {
	if (testStatus == 0) {
		printf("Test ready\n\n");
		testStatus = 1;
	} else {

		int state = (LPC_GPIO2->FIOPIN & (1 << LEDPINB) ? 1 : 0);
		if (state) {
			LPC_GPIO2->FIOCLR |= (1 << LEDPINB);
			LPC_TIM0->TCR = 0x00;
			timerCount = LPC_TIM0->TC;
			timerCount = timerCount/delay;
			printf("time for this test: %d ms\n", timerCount);
			resultBuffer[count] = (double) timerCount;
			count++;
			ready = 1;
			LPC_GPIO2->FIOSET |= (1 << LEDPINA);
			printf("\nAGAIN.\n");
		}
		LPC_GPIOINT->IO2IntClr |= (1 << EINTPIN);
	}
}
