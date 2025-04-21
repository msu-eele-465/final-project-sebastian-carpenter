#include <msp430fr2355.h>
#include <stdint.h>

#include "../../led-bar/src/led_bar.h"
#include "../src/mic.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	init_led_bar();
	init_mic();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

	configure_mic();
	uint8_t sampled_data;

    uint16_t i;
    while (1)
    {
		sample_mic();
		record_mic(sampled_data);

		update_led_bar(sampled_data);

        // delay
        for (i = 0; i < 10000; i++);
    }
}
