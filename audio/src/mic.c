#include <msp430fr2355.h>

#include "./mic.h"

void init_mic(void){
	P1SEL0 |= BIT0;
	P1SEL1 |= BIT0;
}

void configure_mic(void){
	// select 16 conversion cycles
	ADCCTL0 &= !ADCSHT;
	ADCCTL0 |= ADCSHT_2;
	// turn ADC on
	ADCCTL0 |= ADCON;

	// select SMCLK as clock source (1 MHz)
	ADCCTL1 |= ADCSSEL_2;
	// select sampling timer as source of sampling signal
	ADCCTL1 |= ADCSHP;

	// set resolution to 8-bit
	ADCCTL2 &= ~ADCRES;
	ADCCTL2 |= ADCRES_0;

	// set input channel to P1.0
	ADCMCTL0 |= ADCINCH_0;
}
