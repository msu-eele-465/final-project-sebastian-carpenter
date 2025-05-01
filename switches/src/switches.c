#include <msp430fr2355.h>
#include <stdint.h>

#include "./switches.h"

void init_pushbuttons(void){
	// setup P6.4 and P6.5 as inputs with pull-down resistors
	P6SEL0 &= ~(BIT4 | BIT5);
	P6SEL1 &= ~(BIT4 | BIT5);
	P6DIR &= ~(BIT4 | BIT5);
	P6REN |= BIT4 | BIT5;
	P6OUT &= ~(BIT4 | BIT5);
}

void init_rotary_encoders(enum rotary_encoder *rotary_1, enum rotary_encoder *rotary_2){
	// setup P6.0 - P6.1 and P6.2 - P6.3 as inputs with pull-up resistors
	P6SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3);
	P6SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3);
	P6DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);
	P6REN &= ~(BIT0 | BIT1 | BIT2 | BIT3);
	P6OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

	*rotary_1 = HOLD;
	*rotary_2 = HOLD;
}

void check_rotary_encoders(enum rotary_encoder *rotary_1, enum rotary_encoder *rotary_2){
	// check rotary encoder 1, P6.0 (A) P6.1 (B)
	// both A and B low, rotation completed
	if (!(P6IN & 0x03))
	{
		if (*rotary_1 == CW_START)
		{
			*rotary_1 = CW;
		}
		else if (*rotary_1 == CCW_START)
		{
			*rotary_1 = CCW;
		}
	}
	// A low, CW_START occuring
	else if (!(P6IN & 0x01))
	{
		*rotary_1 = CW_START;
	}
	// B low, CCW_START occuring
	else if (!(P6IN & 0x02))
	{
		*rotary_1 = CCW_START;
	}

	// check rotary encoder 2, P6.2 (A) P6.3 (B)
	// both A and B low, rotation completed
	if (!(P6IN & 0x0C))
	{
		if (*rotary_2 == CW_START)
		{
			*rotary_2 = CW;
		}
		else if (*rotary_2 == CCW_START)
		{
			*rotary_2 = CCW;
		}
	}
	// A low, CW_START occuring
	else if (!(P6IN & 0x04))
	{
		*rotary_2 = CW_START;
	}
	// B low, CCW_START occuring
	else if (!(P6IN & 0x08))
	{
		*rotary_2 = CCW_START;
	}
}
