#include "../src/i2c.h"
#include "intrinsics.h"
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

    init_i2c_pins();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    uint8_t buffer[256] = {0};
    configure_i2c_slave(0x48, buffer, 16);

    __enable_interrupt();

    uint16_t i;
    while(1)
    {
        // toggle heartbeat
        if(buffer[0] == 1)
        {
            P2OUT ^= BIT0;
        }

        for(i = 0; i < 10000; i++);
    }
}
