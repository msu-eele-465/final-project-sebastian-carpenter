#ifndef SWITCHES_H
#define SWITCHES_H


/* --- pushbuttons --- */


// setup P6.4, P6.5 as inputs for the pushbuttons: start/stop, mode
// pushbuttons are active high
void init_pushbuttons(void);


/* --- rotary encoder --- */


// an enum encapsulating the state of a rotary encoder
// *_START represents the beginning of a CW or CCW rotation
// 	but where the rotation has not completed
enum rotary_encoder {
	HOLD, CW_START, CCW_START, CW, CCW
};

// setup P6.0 - P6.1 and P6.2 - P6.3 as inputs for the rotary encoders
// the rotary encoders passed in will be set to the HOLD state
void init_rotary_encoders(enum rotary_encoder *rotary_1, enum rotary_encoder *rotary_2);

// updates rotary encoders with the observed state
void check_rotary_encoders(enum rotary_encoder *rotary_1, enum rotary_encoder *rotary_2);

#endif
