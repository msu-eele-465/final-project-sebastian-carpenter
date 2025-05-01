#include <msp430fr2355.h>
#include <stdint.h>

#include "./accelerometer.h"

// the packet to send when initializing the accelerometer
//	the first value is the register address
// this packet will turn off sleep mode
const uint8_t ACCELEROMETER_INIT_PACKET[2] = {107, 0x00};

// stores data during Rx ISR
// accelerometer (59, 64):
//  X_H, X_L, Y_H, Y_L, Z_H, Z_L
// gyroscope (67, 72):
//  X_H, X_L, Y_H, Y_L, Z_H, Z_L
uint8_t accelerometer_data[12];
uint8_t accelerometer_index;

void init_accelerometer_start(void)
{
	// put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;

    // configure eUSCI_B0, for communication with MPU-6050 (GY-521 chip)
    // 1MHz clock
	UCB0CTLW0 |= UCSSEL__SMCLK;
    // 1MHz / 10 = 100kHz
	UCB0BRW = 10;

	// I2C mode
    UCB0CTLW0 |= UCMODE_3;
	// master mode
    UCB0CTLW0 |= UCMST;
	// Tx mode
    UCB0CTLW0 |= UCTR;
	// slave address = 0x68 (GY-521 with pulldown on AD0)
    UCB0I2CSA = 0x69;

	// auto STOP when UCB0TBCNT reached
    UCB0CTLW1 |= UCASTP_2;
	// number of bytes in packet
    UCB0TBCNT = sizeof ACCELEROMETER_INIT_PACKET;

	// pin configuration

	// I2C, SCL (clock), P1.3
    P1SEL1 &= ~BIT3;
    P1SEL0 |= BIT3;

    // I2C, SDA (data), P1.2
    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;
}

void init_accelerometer_end(void)
{
	// take eUSCI_B0 out of software reset
    UCB0CTLW0 &= ~UCSWRST;

    // enable Tx interrupt
    UCB0IFG &= ~UCTXIFG0;
    UCB0IE |= UCTXIE0;

    // enable Rx interrupt
    UCB0IFG &= ~UCRXIFG0;
    UCB0IE |= UCRXIE0;
}

void accelerometer_send_init(void)
{
	uint8_t delay;

	// transmit current time to accelerometer
	// in case the transmission fails this loop will resend it
    while((UCB0IFG & UCSTPIFG) == 0){
		// generate START condition
		accelerometer_index = 0;
        UCB0CTLW0 |= UCTXSTT;

        // delay (~100us)
        for(delay = 0; delay < 1000; delay++);
    }

	// clear stop flag, since transmission has been received
    UCB0IFG &= ~UCSTPIFG;
}
