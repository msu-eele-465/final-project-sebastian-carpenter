#include <msp430fr2355.h>

#include "./timer_interrupts.h"

void init_timer_interrupts(void)
{
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

	// 1MHz / 200Hz = 5kHz
	// CCR0 is for sampling the mic and speaker 1
	TB0CCR0 = 50;
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
