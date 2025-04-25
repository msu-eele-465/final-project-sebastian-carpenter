#include <msp430fr2355.h>
#include <stdint.h>

// audio
#include "../audio/src/mic.h"
#include "../audio/src/speaker.h"

// state
#include "../lcd/src/lcd.h"
#include "../led-bar/src/led_bar.h"
#include "../switches/src/switches.h"


// each location is 208 bytes and there are 4 of them
#define LOCATION_SIZE 208
#define LOCATION_AMNT 4


/* --- important variables --- */


enum system_mode_enum {
    LOCKED, RECORD, RECORDING, PLAYBACK, PLAYING
};


// stores the state of the system
// this can be changed by the main loop or interrupts
static enum system_mode_enum system_mode = LOCKED;

static uint8_t location_1 = 0, location_2 = 0;


/* --- important functions --- */


// update the LCD with the relevant mode and location information
// update the status LED with recording state
// update the LED bar with the mic input strength
void _update_state(void);


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

    // configure hearbeat and status LED as outputs
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 &= ~(BIT0 | BIT1);
    P2OUT &= ~(BIT0 | BIT1);
    P2DIR |= BIT0 | BIT1;

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;


	// stores the movement of the rotary encoders whether CW or CCW
	// because it is done using polling each rotary encoder should
	// 	NOT be turned quickly
	static enum rotary_encoder rotary_1, rotary_2;
	init_rotary_encoders(&rotary_1, &rotary_2);


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
				system_mode = RECORDING;

				// set status LED to indicate recording
				P2OUT |= BIT1;
			}
			else if (system_mode == RECORDING)
			{
				system_mode = RECORD;

				// clear status LED to indicate not recording
				P2OUT &= ~BIT1;
			}
			else if (system_mode == PLAYBACK)
			{
				system_mode = PLAYING;
			}
			else
			{
				system_mode = PLAYBACK;
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


		check_rotary_encoders(&rotary_1, &rotary_2);

		// update location_1 and location_2 based on rotary encoders
		//	each location is capped at 3 (which is 4 slots)
		// recall that unsigned ints have defined overflow behavior
		//	so -1 is 255, therefore these operations are well-defined
		if (rotary_1 == CW)
		{
			location_1 = (location_1 + 1) & 0x03;
			rotary_1 = HOLD;
			_update_state();
		}
		else if (rotary_1 == CCW)
		{
			location_1 = (location_1 - 1) & 0x03;
			rotary_1 = HOLD;
			_update_state();
		}

		if (rotary_2 == CW)
		{
			location_2 = (location_2 + 1) & 0x03;
			rotary_2 = HOLD;
			_update_state();
		}
		else if (rotary_2 == CCW)
		{
			location_2 = (location_2 - 1) & 0x03;
			rotary_2 = HOLD;
			_update_state();
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
	const char *playback_line = "MODE: playback  ";

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
	else if (system_mode == PLAYBACK)
	{
		location_line[L1_START] = '0' + location_1;
		location_line[L2_START] = '0' + location_2;

		lcd_print_line(playback_line, 0);
		lcd_print_line(location_line, 1);
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

