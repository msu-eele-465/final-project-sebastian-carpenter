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

    // results: transition time about 1.5 us
    //  transition time same either direction
    // With all these instructions is takes about 7us before
    //  it starts to switch to the new value
    //
    // analysis: would work up to 100 kHz
    //  more than sufficient for my needs
    while (1)
    {
        update_speaker_1(0);
        update_speaker_2(0);

        update_speaker_1(-1);
        update_speaker_2(-1);
    }
}
