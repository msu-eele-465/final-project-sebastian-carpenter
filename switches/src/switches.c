#include <msp430fr2355.h>

#include "./switches.h"

void init_pushbuttons(void){
	// seupt P6.4 and P6.5 as inputs with pull-down resistors
	P6SEL0 &= ~(BIT4 | BIT5);
	P6SEL1 &= ~(BIT4 | BIT5);
	P6DIR &= ~(BIT4 | BIT5);
	P6REN |= BIT4 | BIT5;
	P6OUT &= ~(BIT4 | BIT5);
}
