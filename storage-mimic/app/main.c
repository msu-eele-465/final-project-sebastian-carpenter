#include "intrinsics.h"
#include <msp430fr2310.h>
#include <stdint.h>

#include "../../i2c/src/i2c.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // hearbeat LED
    P1SEL0 &= !BIT0;
    P1SEL1 &= !BIT0;
    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    init_i2c_pins();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    // 832 / 16 = 52
    uint8_t buffer[832];
    configure_i2c_slave(0x48, buffer, 16);

    __enable_interrupt();

    volatile uint8_t previous_value = *buffer;
    while (1)
    {
        // toggle heartbeat if SCL is low
        if (*buffer != previous_value)
        {
            previous_value = *buffer;
            P1OUT ^= BIT0;
        }
    }
}
