#include <msp430fr2355.h>

#include "./storage_mimic_2355.h"

void init_storage_mimic(void){
	// set P2.0 - P2.7 as digital pins
	P2SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
	P2SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

	// setup P5.0, P5.1 - P5.2 as outputs
	P5SEL0 &= ~(BIT0 | BIT1 | BIT2);
	P5SEL1 &= ~(BIT0 | BIT1 | BIT2);
	P5OUT &= ~(BIT0 | BIT1 | BIT2);
	P5DIR |= BIT0 | BIT1 | BIT2;
}
