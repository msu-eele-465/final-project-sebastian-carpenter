#ifndef MIC_H
#define MIC_H

// setup the ADC pin to capture signals from the mic P1.0
// ---> run before turning on GPIO
void init_mic(void);

// configure the ADC appropriately for sampling from the mic
// ---> run after turning on GPIO
void configure_mic(void);

// enable and start ADC conversion
#define sample_mic() ADCCTL0 |= ADCENC | ADCSC

// get sampled value from ADC
#define record_mic(recorded_value) while ((ADCIFG & ADCIFG0) == 0);\
									recorded_value = ADCMEM0

#endif
