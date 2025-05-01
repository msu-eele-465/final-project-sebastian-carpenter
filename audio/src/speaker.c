#include <msp430fr2355.h>
#include "./speaker.h"

void init_speaker_1(void){
	P1SEL0 |= BIT1;
	P1SEL1 |= BIT1;
}

void init_speaker_2(void){
	P1SEL0 |= BIT5;
	P1SEL1 |= BIT5;
}

void configure_speaker_1(void){
	// DAC reference DVCC
	// DAC update on SAC0DACDAT register update
	SAC0DAC |= DACSREF_0 | DACLSEL_0 ;
	// set DAC to 0v
	SAC0DAT = 0;
	// enable DAC
	SAC0DAC |= DACEN;

	// select 12-bit DAC source for non-inverting input
	// and select PGA for inverting input
	SAC0OA |= PSEL_1 | PMUXEN | NSEL_1 | NMUXEN;
	// high power / fast mode
	SAC0OA |= OAPM_0;
	// select gain of 1 and (OA as buffer mode) ->
	//	basically a closed loop amplifier, I think
	SAC0PGA |= GAIN0 | MSEL_1;

	// enable SAC and OA
	SAC0OA |= SACEN + OAEN;
}

void configure_speaker_2(void){
	// DAC reference DVCC
	// DAC update on SAC0DACDAT register update
	SAC1DAC |= DACSREF_0 | DACLSEL_0 ;
	// set DAC to 0v
	SAC1DAT = 0;
	// enable DAC
	SAC1DAC |= DACEN;

	// select 12-bit DAC source for non-inverting input
	// and select PGA for inverting input
	SAC1OA |= PSEL_1 | PMUXEN | NSEL_1 | NMUXEN;
	// high power / fast mode
	SAC1OA |= OAPM_0;
	// select gain of 1 and (OA as buffer mode) ->
	//	basically a closed loop amplifier, I think
	SAC1PGA |= GAIN0 | MSEL_1;

	// enable SAC and OA
	SAC1OA |= SACEN + OAEN;
}
