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

    configure_i2c_master();

    __enable_interrupt();

    uint16_t i;
    uint8_t buffer[16] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

    while(1)
    {
        // toggle heartbeat
        P2OUT ^= BIT0;
        i2c_write(0x48, 0, buffer, 16);

        for(i = 0; i < 10000; i++);

        // this reads 7 bytes because that is the limit of the AD2
        i2c_read(0x48, 0, buffer, 7);

        // delay
        for(i = 0; i < 10000; i++);
    }
}
