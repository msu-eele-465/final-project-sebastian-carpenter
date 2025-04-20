#include <msp430fr2355.h>
#include <stdint.h>

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // hearbeat LED
    P2SEL0 &= !BIT0;
    P2SEL1 &= !BIT0;
    P2OUT &= ~BIT0;
    P2DIR |= BIT0;

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    uint16_t i;
    while(1)
    {
        // toggle heartbeat
        P2OUT ^= BIT0;

        // delay
        for(i = 0; i < 10000; i++);
    }
}
