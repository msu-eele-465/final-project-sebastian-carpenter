#include "src/led_bar.h"
#include <msp430fr2355.h>
#include <stdint.h>

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    init_led_bar();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    uint16_t i, j;
    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 10000; j++);
    }

    update_led_bar(-1);

    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 10000; j++);
    }

    update_led_bar(3);

    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 10000; j++);
    }

    update_led_bar(1020);
}
