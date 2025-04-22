#include <msp430fr2355.h>
#include <stdint.h>
#include "intrinsics.h"

#include "../src/speaker.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	init_speaker_1();
    init_speaker_2();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

	configure_speaker_1();
    configure_speaker_2();

    __enable_interrupt();

    uint16_t i, j;
    while (1)
    {
        update_speaker_1(856);
        update_speaker_2(856);

        // delay
        for (j = 0; j < 100; j++)
        {
            for (i = 0; i < 10000; i++);
        }

        update_speaker_1(2098);
        update_speaker_2(2098);

        // delay
        for (j = 0; j < 100; j++)
        {
            for (i = 0; i < 10000; i++);
        }

        update_speaker_1(3339);
        update_speaker_2(3339);

        // delay
        for (j = 0; j < 100; j++)
        {
            for (i = 0; i < 10000; i++);
        }
    }
}
