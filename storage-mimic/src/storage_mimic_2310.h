#ifndef STORAGE_MIMIC_2310_H
#define STORAGE_MIMIC_2310_H

// this is meant to be used alongside storage_mimic_2355.h
// most 'functions' are macros becuase speed is prioritized
//
// this parallel protocol is similar to I2C and is thus called I3C
//  incredibly-inter-integrated circuit


/* --- pin descriptions --- */


// P2.0
// buffer 1 -> 0
// buffer 2 -> 1

// P2.6
// write mode -> 0
// read mode  -> 1
//
// NOTE: when write mode is called after read mode a register is
// 	expected for the first byte instead of a new value to store

// P2.7
// interrupt on transition from 0 to 1


/* --- functions --- */


// setup P1.0 - P1.7 as inputs, thus init to read mode
// setup P2.0, P2.6 - P2.7 as inputs, with an interrupt on P2.7
// 	P2.0 -> buffer select bit, ignored in write mode
//  P2.6 -> read/write bit
//  P2.7 -> interrupt bit, used for preventing erroneous
//      data capture and access, low-to-high transition
void init_storage_mimic(void);

// set P1.0 - P1.7 as outputs
#define storage_mimic_write_mode() P1DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7

// set P1.0 - P1.7 as inputs
#define storage_mimic_read_mode() P1DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)

// write_mode() must be called previously
// update P1.0 - P1.7 with new data
#define storage_mimic_update(data) P1OUT = data

// read_mode() must be called previously
// read data from P1.0 - P1.7
#define storage_mimic_read(data) data = P1IN

#endif
