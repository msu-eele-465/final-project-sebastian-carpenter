#include <msp430fr2355.h>
#include <stdint.h>

#include "../src/switches.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	// to be toggled by rotary encoders
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 &= ~(BIT0 | BIT1);
    P2OUT &= ~(BIT0 | BIT1);
    P2DIR |= BIT0 | BIT1;

	enum rotary_encoder dereferenced_rotary_1;
	enum rotary_encoder dereferenced_rotary_2;
	enum rotary_encoder *rotary_1 = &dereferenced_rotary_1;
	enum rotary_encoder *rotary_2 = &dereferenced_rotary_2;

	init_rotary_encoders(rotary_1, rotary_2);

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    uint16_t i;
    while (1)
    {
        // delay
        for (i = 0; i < 10000; i++);

		check_rotary_encoders(rotary_1, rotary_2);

		// update LED 1 and 2 based on rotation of rotary encoders
		if (dereferenced_rotary_1 == CW)
		{
			P2OUT |= BIT0;
			dereferenced_rotary_1 = HOLD;
		}
		else if (dereferenced_rotary_1 == CCW)
		{
			P2OUT &= ~BIT0;
			dereferenced_rotary_1 = HOLD;
		}

		if (dereferenced_rotary_2 == CW)
		{
			P2OUT |= BIT1;
			dereferenced_rotary_2 = HOLD;
		}
		else if (dereferenced_rotary_2 == CCW)
		{
			P2OUT &= ~BIT1;
			dereferenced_rotary_2 = HOLD;
		}
    }
}
