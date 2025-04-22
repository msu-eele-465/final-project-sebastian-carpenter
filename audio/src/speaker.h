#ifndef SPEAKER_H
#define SPEAKER_H

// setup the SAC pin to output signals to the speaker circuit
// P1.1 -> SAC0, P1.5 -> SAC1
// ---> run before turning on GPIO
void init_speaker_1(void);
void init_speaker_2(void);

// configure the SAC appropriately to output signals
// ---> run after turning on GPIO
void configure_speaker_1(void);
void configure_speaker_2(void);

// update DAC with given value
// subsequently, the SAC will output it
#define update_speaker_1(voltage) SAC0DAT = voltage
#define update_speaker_2(voltage) SAC1DAT = voltage

#endif
