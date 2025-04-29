#include "intrinsics.h"
#include <msp430fr2355.h>
#include <stdint.h>

#include "../src/led_bar.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    init_led_bar();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    while (1)
    {
        __delay_cycles(1000000);

        update_led_bar(-1);

        __delay_cycles(1000000);

        update_led_bar(0x07);

        __delay_cycles(1000000);

        update_led_bar(0xF0);
    }
}
