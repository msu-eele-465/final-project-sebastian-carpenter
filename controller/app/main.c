#include <msp430fr2355.h>
#include <stdint.h>

// audio
#include "../../audio/src/mic.h"
#include "../../audio/src/speaker.h"

// state
#include "../../lcd/src/lcd.h"
#include "../../led-bar/src/led_bar.h"
#include "../../switches/src/switches.h"

// update
#include "../src/timer_interrupts.h"
#include "intrinsics.h"


// each location is 224 bytes and there are 4 of them
#define LOCATION_SIZE 224
#define LOCATION_SIZE_ADDRESSABLE 14
#define LOCATION_MAX 0x03


/* --- important variables --- */


enum system_mode_enum {
    LOCKED, RECORD, RECORDING, PLAYBACK, PLAYING
};


// stores the state of the system
// this can be changed by the main loop or interrupts
static enum system_mode_enum system_mode = LOCKED;

// these locations are references to a portion of the
// 	memory for the recorded/recording audio
static uint8_t location_1 = 0, location_2 = 0;

// used for sending and receiving audio data from storage
static uint8_t buffer_1_a[16];
static uint8_t buffer_1_b[16];
static uint8_t buffer_2_a[16];
static uint8_t buffer_2_b[16];
static uint8_t buffer_1_index = 0;
static uint8_t buffer_2_index = 0;

// TEMPORARY: used for mimicing i2c storage right now
static uint8_t storage[LOCATION_SIZE * 4] = {0};
static uint8_t storage_index_1, storage_index_2;
static uint8_t storage_index_1_max, storage_index_2_max;


/* --- important functions --- */


// update the LCD with the relevant mode and location information
// update the status LED with recording state
// update the LED bar with the mic input strength
void _update_state(void);

// send the provided buffer to i2c storage
void _send_to_storage(uint8_t buffer_to_store[16]);

// read i2c storage into the provided buffer
// index_number -> 0 is buffer_1, 1 is buffer_2
void _receive_from_storage(uint8_t buffer_to_store[16], uint8_t index_number);


/* --- main loop --- */


int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    init_mic();
    init_speaker_1();
    init_speaker_2();

    lcd_init();
    init_led_bar();
    init_pushbuttons();

	init_timer_interrupts();

    // configure hearbeat and status LED as outputs
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 &= ~(BIT0 | BIT1);
    P2OUT &= ~(BIT0 | BIT1);
    P2DIR |= BIT0 | BIT1;

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

	configure_mic();
	configure_speaker_1();
	configure_speaker_2();


	// stores the movement of the rotary encoders whether CW or CCW
	// because it is done using polling each rotary encoder should
	// 	NOT be turned quickly
	static enum rotary_encoder rotary_1, rotary_2;
	init_rotary_encoders(&rotary_1, &rotary_2);

	__enable_interrupt();


    while (1)
    {
        // get password


        while (system_mode == LOCKED)
        {
            // MODE SHOULD START IN LOCKED STATE
            system_mode = RECORD;
            // THIS IS TEMPORARY
			
			_update_state();
        }


		// get pushbutton press


		// toggle recording/playback
		// 	start/stop	-> pressed
		// 	mode		-> not pressed
        if (P6IN & BIT4 && !(P6IN & BIT5))
        {
            if (system_mode == RECORD)
			{
				sample_mic();

				system_mode = RECORDING;

				storage_index_1 = location_1 * LOCATION_SIZE_ADDRESSABLE;
				storage_index_1_max = storage_index_1 + LOCATION_SIZE_ADDRESSABLE;

				// recording can occur, turn on interrupt
				set_mic_interrupt();
			}
			else if (system_mode == RECORDING)
			{
				system_mode = RECORD;

				// recording has stopped, turn off interrupt
				clear_mic_interrupt();
			}
			else if (system_mode == PLAYBACK)
			{
				system_mode = PLAYING;

				storage_index_1 = location_1 * LOCATION_SIZE_ADDRESSABLE;
				storage_index_2 = location_2 * LOCATION_SIZE_ADDRESSABLE;
				storage_index_1_max = storage_index_1 + LOCATION_SIZE_ADDRESSABLE;
				storage_index_2_max = storage_index_2 + LOCATION_SIZE_ADDRESSABLE;

				// retrieve 4 initial buffers from storage
				_receive_from_storage(buffer_1_a, 0);
				__delay_cycles(1000);
				_receive_from_storage(buffer_1_b, 0);
				__delay_cycles(1000);
				_receive_from_storage(buffer_2_a, 1);
				__delay_cycles(1000);
				_receive_from_storage(buffer_2_b, 1);

				// playing has started, turn on interrupts
				set_speaker_interrupts();
			}
			else if (system_mode == PLAYING)
			{
				system_mode = PLAYBACK;

				// playing has stopped, turn off interrupts
				clear_speaker_interrupts();
			}

			_update_state();
        }
		// toggle mode
		// 	start/stop	-> not pressed
		// 	mode		-> pressed
		else if (!(P6IN & BIT4) && P6IN & BIT5)
		{
			// the system must not be performing an action when
			// 	switching modes (RECORDING, PLAYING)
			if (system_mode == RECORD)
			{
				system_mode = PLAYBACK;
			}
			else if (system_mode == PLAYBACK)
			{
				system_mode = RECORD;
			}

			_update_state();
		}
		else if (P6IN & BIT4 && P6IN & BIT5)
		{
			// the system must not be performing an action when
			// 	becoming locked (RECORDING, PLAYING)
			if (system_mode == RECORD || system_mode == PLAYBACK)
			{
				system_mode = LOCKED;
			}

			_update_state();
		}


		// get location updates


		if (system_mode == PLAYBACK || system_mode == RECORD)
		{
			check_rotary_encoders(&rotary_1, &rotary_2);

			// update location_1 and location_2 based on rotary encoders
			//	each location is capped at 3 (which is 4 slots)
			// recall that unsigned ints have defined overflow behavior
			//	so -1 is 255, therefore these operations are well-defined
			if (rotary_1 == CW)
			{
				location_1 = (location_1 + 1) & LOCATION_MAX;
				rotary_1 = HOLD;
				_update_state();
			}
			else if (rotary_1 == CCW)
			{
				location_1 = (location_1 - 1) & LOCATION_MAX;
				rotary_1 = HOLD;
				_update_state();
			}

			if (rotary_2 == CW)
			{
				location_2 = (location_2 + 1) & LOCATION_MAX;
				rotary_2 = HOLD;
				_update_state();
			}
			else if (rotary_2 == CCW)
			{
				location_2 = (location_2 - 1) & LOCATION_MAX;
				rotary_2 = HOLD;
				_update_state();
			}
		}
    }
}


/* --- function implementations --- */


#define X_START 3
#define Y_START 8
#define Z_START 13
#define L1_START 6
#define L2_START 13

void _update_state(void){

	// update LCD

	const char *locked_line_1 = "MODE: locked    ";
	const char *record_line = "MODE: record    ";
	const char *recording_line = "MODE: recording ";
	const char *playback_line = "MODE: playback  ";
	const char *playing_line = "MODE: playing   ";

	static char locked_line_2[16] = " x=xx y=xx z=xx ";
	static char location_line[16] = "  L1: X  L2: X  ";

	if (system_mode == LOCKED)
	{
		lcd_print_line(locked_line_1, 0);
		lcd_print_line(locked_line_2, 1);
	}
	else if (system_mode == RECORD)
	{
		location_line[L1_START] = '0' + location_1;
		location_line[L2_START] = '0' + location_2;

		lcd_print_line(record_line, 0);
		lcd_print_line(location_line, 1);
	}
	else if (system_mode == RECORDING)
	{
		lcd_print_line(recording_line, 0);
	}
	else if (system_mode == PLAYBACK)
	{
		location_line[L1_START] = '0' + location_1;
		location_line[L2_START] = '0' + location_2;

		lcd_print_line(playback_line, 0);
		lcd_print_line(location_line, 1);
	}
	else if (system_mode == PLAYING)
	{
		lcd_print_line(playing_line, 0);
	}

	// update status LED
	
	if (system_mode == RECORD)
	{
		P2OUT &= ~BIT1;
	}
	else if (system_mode == RECORDING)
	{
		P2OUT |= BIT1;
	}

	// delay for 200ms
	// this prevents a switch from being pressed multiple times on accident
	__delay_cycles(200000);
}

void _send_to_storage(uint8_t buffer_to_store[16]){
	uint8_t *storage_pointer = storage;
	storage_pointer += (uint16_t)(storage_index_1) * 16;

	uint8_t i;
	for (i = 16; i > 0; i--)
	{
		*storage_pointer++ = *buffer_to_store++;
	}

	storage_index_1++;
}

void _receive_from_storage(uint8_t buffer_to_store[16], uint8_t index_number){
	// selects the appropriate location in memory
	uint8_t *storage_pointer = storage;
	storage_pointer += (index_number) ? ((uint16_t)(storage_index_2) * 16) : ((uint16_t)(storage_index_1) * 16);

	uint8_t i;
	for (i = 16; i > 0; i--)
	{
		*buffer_to_store++ = *storage_pointer++;
	}

	(index_number) ? (storage_index_2++) : (storage_index_1++);
}


/* --- interrupts --- */


// which buffer is in use
enum buffer_mode {
	ONE, TWO
};

// CCR0
#pragma vector=TIMER0_B0_VECTOR
__interrupt void speaker_1_and_mic_ISR(void)
{
	static enum buffer_mode buffer_1_mode = ONE;

	if (system_mode == RECORDING)
	{
		// capture audio from the mic and store in the correct buffer
		// additionally, sample preemptively for the next interrupt
		if (buffer_1_mode == ONE)
		{
			record_mic(buffer_1_a[buffer_1_index]);
			sample_mic();
			buffer_1_index++;

			// send full buffer to storage
			if (buffer_1_index >= 16)
			{
				_send_to_storage(buffer_1_a);
				buffer_1_index = 0;
				buffer_1_mode = TWO;
			}
		}
		else
		{
			record_mic(buffer_1_b[buffer_1_index]);
			sample_mic();
			buffer_1_index++;

			// send full buffer to storage
			if (buffer_1_index >= 16)
			{
				_send_to_storage(buffer_1_b);
				buffer_1_index = 0;
				buffer_1_mode = ONE;
			}
		}

		// stop recording if the location is filled
		if (storage_index_1 >= storage_index_1_max)
		{
			clear_mic_interrupt();
			system_mode = RECORD;
			_update_state();
		}
	}
	else if (system_mode == PLAYING)
	{
		// cycle buffers to play from
		if (buffer_1_mode == ONE)
		{
			update_speaker_1(buffer_1_a[buffer_1_index] << 4);
			buffer_1_index++;

			// receive full buffer from storage
			if (buffer_1_index >= 16)
			{
				_receive_from_storage(buffer_1_a, 0);
				buffer_1_mode = TWO;
				buffer_1_index = 0;
			}
		}
		else
		{
			update_speaker_1(buffer_1_b[buffer_1_index] << 4);
			buffer_1_index++;

			// receive full buffer from storage
			if (buffer_1_index >= 16)
			{
				_receive_from_storage(buffer_1_b, 0);
				buffer_1_mode = ONE;
				buffer_1_index = 0;
			}
		}

		// if at max reset storage index to beginning
		// 	so audio keeps playing
		if (storage_index_1 >= storage_index_1_max)
		{
			storage_index_1 = location_1 * LOCATION_SIZE_ADDRESSABLE;
		}
	}

	TB0CCTL0 &= ~CCIFG;
}

// CCR1
#pragma vector=TIMER0_B1_VECTOR
__interrupt void speaker_2_ISR(void)
{
	static enum buffer_mode buffer_2_mode = ONE;

	// cycle buffers to play from
	if (buffer_2_mode == ONE)
	{
		update_speaker_2(buffer_2_a[buffer_2_index] << 4);
		buffer_2_index++;

		// receive full buffer from storage
		if (buffer_2_index >= 16)
		{
			_receive_from_storage(buffer_2_a, 1);
			buffer_2_mode = TWO;
			buffer_2_index = 0;
		}
	}
	else
	{
		update_speaker_2(buffer_2_b[buffer_2_index] << 4);
		buffer_2_index++;

		// receive full buffer from storage
		if (buffer_2_index >= 16)
		{
			_receive_from_storage(buffer_2_b, 1);
			buffer_2_mode = ONE;
			buffer_2_index = 0;
		}
	}

	// if at max reset storage index to beginning
	// 	so audio keeps playing
	if (storage_index_2 >= storage_index_2_max)
	{
		storage_index_2 = location_2 * LOCATION_SIZE_ADDRESSABLE;
	}

	TB0CCTL1 &= ~CCIFG;
}
