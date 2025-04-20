#include <msp430fr2355.h>
#include <stdint.h>

#include "../src/switches.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	// to be toggled by pushbuttons
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 &= ~(BIT0 | BIT1);
    P2OUT &= ~(BIT0 | BIT1);
    P2DIR |= BIT0 | BIT1;

	init_pushbuttons();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    uint16_t i;
    while (1)
    {
        // delay
        for (i = 0; i < 10000; i++);

		if (P6IN & 0x10)
		{
			P2OUT ^= BIT0;
		}

		if (P6IN & 0x20)
		{
			P2OUT ^= BIT1;
		}
    }
}
