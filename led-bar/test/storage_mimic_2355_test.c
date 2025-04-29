#include <msp430fr2355.h>
#include <stdint.h>

#include "../src/led_bar.h"
#include "../../storage-mimic/src/storage_mimic_2355.h"
#include "intrinsics.h"
#include "../../switches/src/switches.h"

// this test is in the led-bar directory becuase it is supposed
//	to be compiled on the 2355, not the 2310

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    init_led_bar();
	init_storage_mimic();
	init_pushbuttons();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

	__enable_interrupt();

	uint8_t data, trash;
	while (1)
	{
		/* -------------- reading working
		storage_mimic_read_mode();
		__delay_cycles(10000);
		storage_mimic_read(data);
		__delay_cycles(10000);

		update_led_bar(data);
		__delay_cycles(1000000);*/

		/* -------------- buffer 1 working
		storage_mimic_read_mode();
		storage_mimic_read(trash);

		storage_mimic_write_mode();
		storage_mimic_update(0x00);
		storage_mimic_update(0x0F);
		storage_mimic_update(0xF0);

		storage_mimic_read_mode();
		storage_mimic_read(trash);

		storage_mimic_write_mode();
		storage_mimic_update(0x00);
		storage_mimic_read_mode();

		storage_mimic_read(data);
		update_led_bar(data);
		__delay_cycles(1000000);

		storage_mimic_read(data);
		update_led_bar(data);
		__delay_cycles(1000000);*/

		storage_mimic_select_buffer_1();
		storage_mimic_read_mode();
		storage_mimic_read(trash);

		storage_mimic_write_mode();
		storage_mimic_update(0x00);
		storage_mimic_update(0x0F);
		storage_mimic_update(0x0F);

		storage_mimic_select_buffer_2();
		storage_mimic_read_mode();
		storage_mimic_read(trash);

		storage_mimic_write_mode();
		storage_mimic_update(0x01);
		storage_mimic_update(0xF0);
		storage_mimic_update(0xF0);

		storage_mimic_read_mode();

		storage_mimic_select_buffer_1();
		storage_mimic_read(data);
		update_led_bar(data);
		__delay_cycles(1000000);

		storage_mimic_select_buffer_2();
		storage_mimic_read(data);
		update_led_bar(data);
		__delay_cycles(1000000);
	}
}
