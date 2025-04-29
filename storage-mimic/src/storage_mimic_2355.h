#ifndef STORAGE_MIMIC_2355_H
#define STORAGE_MIMIC_2355_H

// this is meant to be used alongside storage_mimic_2310.h
// most 'functions' are macros becuase speed is prioritized
//
// this parallel protocol is similar to I2C and is thus called I3C
//  incredibly-inter-integrated circuit


/* --- pin descriptions --- */


// P5.0
// buffer 1 -> 0
// buffer 2 -> 1

// P5.1
// write mode -> 0
// read mode  -> 1
//
// NOTE: when selecting a new register with write mode, a value
// 	should be read in advance. This enforces that the 2310 expects
// 	a register instead of another value to store

// P5.2
// 2310 interrupt on transition from 0 to 1


/* --- functions --- */



// setup P2.0 - P2.7 as digital pins
// setup P5.0 - P5.2 as outputs, thus init to write mode
//  P5.0 -> buffer select bit
//  P5.1 -> read/write bit
//  P5.2 -> interrupt bit, used for preventing erroneous
//      data capture and access, low-to-high transition
void init_storage_mimic(void);

// set P2.0 - P2.7 as outputs
// change P5.1 to write mode
#define storage_mimic_write_mode() P5OUT &= ~BIT1;\
                                   __delay_cycles(10);\
                                   P2DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7

// set P2.0 - P2.7 as inputs
// change P5.1 to read mode
#define storage_mimic_read_mode() P2DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);\
                                  P5OUT |= BIT1

// write_mode() must be called previously
// update P2.0 - P2.7 with new data
#define storage_mimic_update(data) P2OUT = data;\
                                   P5OUT |= BIT2;\
                                   __delay_cycles(75);\
                                   P5OUT &= ~BIT2

// select buffer 1 or 2 to read from
#define storage_mimic_select_buffer_1() P5OUT &= ~BIT0
#define storage_mimic_select_buffer_2() P5OUT |= BIT0

// read_mode() must be called previously
// a buffer should be selected previously
// read data from P2.0 - P2.7
#define storage_mimic_read(data) P5OUT |= BIT2;\
                                 __delay_cycles(75);\
								 P5OUT &= ~BIT2;\
								 data = P2IN

#endif
