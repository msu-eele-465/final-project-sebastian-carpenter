#include <msp430fr2355.h>

#include "./timer_interrupts.h"

void init_timer_interrupts(void)
{
	// TIMER 0


	// clear timer information
	TB0CTL |= TBCLR;

	// choose 1MHz clock
	TB0CTL |= TBSSEL__SMCLK;
	// count up from 0, reset at CCR0
	TB0CTL |= MC__UP;
	// capacity of 2^16
	TB0CTL |= CNTL_0;
	// divide by 1
	TB0CTL |= ID_0;
	// divide by 1
	TB0EX0 |= TBIDEX_0;

	// 1MHz / 50Hz = 20kHz
	// CCR0 is for sampling the mic and outputting to the speakers
	TB0CCR0 = 50;


	// TIMER 1


	// clear timer information
	TB1CTL |= TBCLR;

	// choose 1MHz clock
	TB1CTL |= TBSSEL__SMCLK;
	// count up from 0, reset at CCR0
	TB1CTL |= MC__UP;
	// capacity of 2^16
	TB1CTL |= CNTL_0;
	// divide by 1
	TB1CTL |= ID_0;
	// divide by 1
	TB1EX0 |= TBIDEX_0;

	// 1MHz / 200Hz = 20Hz
	// CCR0 is for updating the audio display while not
	//	PLAYING or RECORDING
	TB1CCR0 = 50000;
}

void clear_mic_interrupt(void)
{
	TB0CCTL0 &= ~CCIE;
}

void set_mic_interrupt(void)
{
	TB0CCTL0 &= ~CCIFG;
	TB0CCTL0 |= CCIE;
}

void clear_speaker_interrupt(void)
{
	TB0CCTL0 &= ~CCIE;
}

void set_speaker_interrupt(void)
{
	TB0CCTL0 &= ~CCIFG;
	TB0CCTL0 |= CCIE;
}

void clear_audio_display_interrupt(void)
{
	TB1CCTL0 &= ~CCIE;
}

void set_audio_display_interrupt(void)
{
	TB1CCTL0 &= ~CCIFG;
	TB1CCTL0 |= CCIE;
}
