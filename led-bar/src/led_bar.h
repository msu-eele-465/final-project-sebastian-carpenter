#ifndef LED_BAR_H
#define LED_BAR_H

#include <stdint.h>

// initialize the appropriate pins as outputs for the LED bar
// pins: P2.6 - P2.7, P3.0 - P3.7
void init_led_bar(void);

// set or clear distinct LEDs on the LED bar according to the pattern
// only the first 10 bits are used
// ordering: P2.6 - P2.7, P3.0 - P3.7
void update_led_bar(uint16_t pattern);

#endif
