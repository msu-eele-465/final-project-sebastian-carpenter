#include <msp430fr2310.h>

#include "./storage_mimic_2310.h"

void init_storage_mimic(void){
	// set P1.0 - P1.7 as inputs
	// no resistor necessary
	P1SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
	P1SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
	storage_mimic_read_mode();

	// setup P2.0, P2.6 - P2.7 as inputs
	P2SEL0 &= ~(BIT0 | BIT6 | BIT7);
	P2SEL1 &= ~(BIT0 | BIT6 | BIT7);
	P2DIR &= ~(BIT0 | BIT6 | BIT7);
	// choose pull down resistors
	P2REN |= BIT0 | BIT6 | BIT7;
	P2OUT &= ~(BIT0 | BIT6 | BIT7);

	// put an interrupt on P2.7
	// triggered on low-to-high transition
	P2IES &= ~BIT7;
	P2IE |= BIT7;
}
