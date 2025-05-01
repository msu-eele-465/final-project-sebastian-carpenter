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

// misc
#include "intrinsics.h"


// each location is 7000 bytes and there are 4 of them
// @20kHz this should be ~350ms of audio data per location
#define LOCATION_SIZE 7000
#define LOCATION_MAX 0x03

// when displaying audio strength, how many samples should
//	be checked to determine amplitude
#define AUDIO_DISPLAY_SIZE 10

// how many poll cycles (~200ms) should elapse before moving
//	into a locked state
#define CYCLES_TO_LOCK 5


/* --- important variables --- */


enum system_mode_enum {
    LOCKED, RECORD, RECORDING, PLAYBACK, PLAYING
};


// stores the state of the system
// this can be changed by the main loop or interrupts
static enum system_mode_enum system_mode = LOCKED;

// hold the mode button to increment this value
// when lock_cycles == CYCLES_TO_LOCK system_mode will
// 	change to LOCKED
static uint8_t lock_cycles;

// these locations are references to a portion of the
// 	memory for the recorded/recording audio
// altered by the rotary encoders
static uint8_t location_1 = 0, location_2 = 0;

#pragma PERSISTENT(storage)
static uint8_t storage[LOCATION_SIZE * 4] = {0};

// storage for audio, variety of variables to increase speed
static uint8_t *storage_pointer_1 = storage;
static uint8_t *storage_pointer_2 = storage;
static uint8_t *storage_pointer_1_start, *storage_pointer_2_start;
static uint16_t storage_index;


/* --- important functions --- */


// update the LCD with the relevant mode and location information
// update the LED bar with the mic input strength
void _update_state(void);

// find the largest sample and display it according to a logarithmic scale
// values checked are inclusive of buffer start and exclusive of buffer_end
void _update_led_bar(uint8_t *buffer_start, uint8_t *buffer_end);


/* --- main loop --- */


#define toggle_heartbeat() P5OUT ^= BIT4

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

    // configure status LED (P5.3) and heartbeat (P5.4) as outputs
    P5SEL0 &= ~(BIT3 | BIT4);
    P5SEL1 &= ~(BIT3 | BIT4);
    P5OUT &= ~(BIT3 | BIT4);
    P5DIR |= BIT3 | BIT4;

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

	configure_mic();
	configure_speaker_1();
	configure_speaker_2();

	// stores the movement of the rotary encoders whether CW or CCW
	static enum rotary_encoder rotary_1, rotary_2;
	init_rotary_encoders(&rotary_1, &rotary_2);

	__enable_interrupt();


	// start displaying audio strength using the interrupt
	sample_mic();
	set_audio_display_interrupt();

    while (1)
    {
        // get password


		while (system_mode == LOCKED)
		{
			// if start/stop pressed exit LOCKED state
			// this is to simulate latching a password from the
			//  accelerometer which is currently not working
			if (P6IN & BIT4 && !(P6IN & BIT5))
			{
				system_mode = RECORD;

				_update_state();

				// delay an extra second to prevent recording on accident
				__delay_cycles(1000000);
			}
			else
			{
				_update_state();
			}
		}


		// get pushbutton press


		// toggle recording/playback
		// 	start/stop	-> pressed
		// 	mode		-> not pressed
        if (P6IN & BIT4 && !(P6IN & BIT5))
        {
            if (system_mode == RECORD)
			{
				clear_audio_display_interrupt();
				sample_mic();

				system_mode = RECORDING;

				P5OUT |= BIT3;

				storage_pointer_1 = storage + (uint16_t)(location_1) * LOCATION_SIZE;
				storage_index = 0;

				// recording can occur, turn on interrupt
				set_mic_interrupt();
			}
			else if (system_mode == RECORDING)
			{
				// recording has stopped, turn off interrupt
				clear_mic_interrupt();

				P5OUT &= ~BIT3;

				system_mode = RECORD;

				// start displaying audio strength using the interrupt
				// the mic interrupt gives a sample that can be used so
				//	sample_mic() does not need to be called
				set_audio_display_interrupt();
			}
			else if (system_mode == PLAYBACK)
			{
				clear_audio_display_interrupt();
				system_mode = PLAYING;

				storage_pointer_1_start = storage + (uint16_t)(location_1) * LOCATION_SIZE;
				storage_pointer_1 = storage_pointer_1_start;

				storage_pointer_2_start = storage + (uint16_t)(location_2) * LOCATION_SIZE;
				storage_pointer_2 = storage_pointer_2_start;

				storage_index = 0;

				// playing has started, turn on interrupt
				set_speaker_interrupt();
			}
			else if (system_mode == PLAYING)
			{
				// playing has stopped, turn off interrupt
				clear_speaker_interrupt();

				system_mode = PLAYBACK;

				// start displaying audio strength using the interrupt
				sample_mic();
				set_audio_display_interrupt();
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

			// the system must not be performing an action when
			// 	becoming locked (RECORDING, PLAYING)
			if (system_mode == RECORD || system_mode == PLAYBACK)
			{
				lock_cycles++;

				if (lock_cycles >= CYCLES_TO_LOCK)
				{
					system_mode = LOCKED;
				}
			}

			_update_state();
		}
		else
		{
			// mode button was not pressed so reset lock_cycles
			// technically start/stop could be pressed to prevent this
			// 	but covering this case does not really matter
			lock_cycles = 0;

			__delay_cycles(20000);
			toggle_heartbeat();
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

		// also update LED bar with recording strength
		// the largest value over AUDIO_DISPLAY_SIZE is displayed
		uint8_t *temp_storage_pointer = storage_pointer_1 - AUDIO_DISPLAY_SIZE;
		if (temp_storage_pointer < storage)
		{
			temp_storage_pointer = storage;
		}

		_update_led_bar(temp_storage_pointer, storage_pointer_1);
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

	// delay for 200ms
	// this prevents a switch from being pressed multiple times on accident
	__delay_cycles(200000);

	toggle_heartbeat();
}

void _update_led_bar(uint8_t *buffer_start, uint8_t *buffer_end){
	// find largest value within provided values
	uint8_t largest = *buffer_start;
	while (++buffer_start < buffer_end)
	{
		if (*buffer_start > largest)
		{
			largest = *buffer_start;
		}
	}

	// update LED bar according to a logarithmic scale
	if (largest > 203)
	{
		largest = 0xFF;
	}
	else if (largest > 161)
	{
		largest = 0x7F;
	}
	else if (largest > 128)
	{
		largest = 0x3F;
	}
	else if (largest > 102)
	{
		largest = 0x1F;
	}
	else if (largest > 81)
	{
		largest = 0x0F;
	}
	else if (largest > 64)
	{
		largest = 0x07;
	}
	else if (largest > 50)
	{
		largest = 0x03;
	}
	else if (largest > 40)
	{
		largest = 0x01;
	}
	else
	{
		largest = 0x00;
	}

	update_led_bar(largest);
}


/* --- interrupts --- */


// CCR0 TIMER0
#pragma vector=TIMER0_B0_VECTOR
__interrupt void speaker_and_mic_ISR(void)
{
	if (system_mode == RECORDING)
	{
		// capture audio from the mic and store it
		// additionally, sample preemptively for the next interrupt
		record_mic(*storage_pointer_1++);
		sample_mic();
		storage_index++;

		// stop recording if the location is filled
		// system state will remain as RECORDING
		// 	this is intentional as it shows the user that they
		// 	exceeded the recording slot's capacity
		if (storage_index >= LOCATION_SIZE)
		{
			clear_mic_interrupt();
			P5OUT &= ~BIT3;
		}
	}
	else if (system_mode == PLAYING)
	{
		update_speaker_1((uint16_t)(*storage_pointer_1++) << 4);
		update_speaker_2((uint16_t)(*storage_pointer_2++) << 4);
		storage_index++;

		// if at max reset storage index to beginning
		// 	so audio keeps playing
		if (storage_index >= LOCATION_SIZE)
		{
			storage_pointer_1 = storage_pointer_1_start;
			storage_pointer_2 = storage_pointer_2_start;
			storage_index = 0;
		}
	}

	TB0CCTL0 &= ~CCIFG;
}

// CCR0 TIMER1
#pragma vector=TIMER1_B0_VECTOR
__interrupt void audio_display_ISR(void)
{
	static uint8_t audio_buffer[AUDIO_DISPLAY_SIZE];
	static uint8_t audio_buffer_index = 0;

	record_mic(audio_buffer[audio_buffer_index]);
	sample_mic();

	if (audio_buffer_index > AUDIO_DISPLAY_SIZE)
	{
		audio_buffer_index = 0;
	}

	// display the largest value over a span of AUDIO_DISPLAY_SIZE
	_update_led_bar(audio_buffer, audio_buffer + AUDIO_DISPLAY_SIZE);

	TB1CCTL0 &= ~CCIFG;
}
