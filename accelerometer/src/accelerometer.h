#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>

// the packet to send when initializing the accelerometer
//	the first value is the register address
extern const uint8_t ACCELEROMETER_INIT_PACKET[2];

// stores data during Rx ISR
// accelerometer (59, 64):
//  X_H, X_L, Y_H, Y_L, Z_H, Z_L
// gyroscope (67, 72):
//  X_H, X_L, Y_H, Y_L, Z_H, Z_L
extern uint8_t accelerometer_data[12];
extern uint8_t accelerometer_index;

// setup I2C for accelerometer
// ---> run before turning on GPIO
void init_accelerometer_start(void);

// turn on I2C for accelerometer
// ---> run after turning on GPIO
void init_accelerometer_end(void);

// send the init packet for the accelerometer
// ---> run after turning on interrupts
void accelerometer_send_init(void);

#endif
