#include <msp430fr2355.h>
#include <stdint.h>

#include "./i2c.h"

static volatile uint8_t master;

static volatile uint8_t transmit_buffer[16];
static volatile uint8_t *receive_buffer;

static volatile uint8_t device_register;

static volatile uint8_t buffer_index;
static volatile uint8_t tx_count;
static volatile uint8_t rx_count;

enum I2C_MODES{
	SEND_DEVICE_REGISTER, TRANSMIT, READ_START, READ
};

static volatile enum I2C_MODES mode;

void init_i2c_pins(void)
{
    P1SEL0 |= BIT2 | BIT3;
    P1SEL1 &= ~(BIT2 | BIT3);
}

void configure_i2c_master(void)
{
    // turn on the software reset
    UCB0CTLW0 |= UCSWRST;
    // i2c master mode, 1MHz clock
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK | UCSYNC;
    // 1MHz / 10 = 100kHz
    UCB0BRW = 10;
    // turn off the software reset
    UCB0CTLW0 &= ~UCSWRST;
    // enable i2c interrupt
    //UCB0IE |= UCNACKIE;

	master = 1;
}

void i2c_write(uint8_t device_address, uint8_t register_address, uint8_t *buffer, uint8_t count)
{
	mode = SEND_DEVICE_REGISTER;

	device_register = register_address;
	buffer_index = 0;
	// this is 255 so that it rolls over once incremented
	// 	otherwise the first byte will be wrong and too many
	//	bytes will be sent
	tx_count = -1;
	rx_count = 0;

	while(++tx_count < count)
	{
		transmit_buffer[tx_count] = buffer[tx_count];
	}

	// set up slave to transmit to
	UCB0I2CSA = device_address;
	// clear pending interrupts
	UCB0IFG &= ~(UCTXIFG0 | UCRXIFG0);
	// disable RX interrupt
	UCB0IE &= ~UCRXIE0;
	// enable TX interrupt
	UCB0IE |= UCTXIE0;

	// transmit mode and send start condition
	UCB0CTLW0 |= UCTR | UCTXSTT;
}

void i2c_read(uint8_t device_address, uint8_t register_address, uint8_t *buffer, uint8_t count)
{
	mode = SEND_DEVICE_REGISTER;

	device_register = register_address;
	buffer_index = 0;
	tx_count = 0;
	rx_count = count;

	receive_buffer = buffer;

	// setup slave to receive from
	UCB0I2CSA = device_address;
	// clear pending interrupts
	UCB0IFG &= ~(UCTXIFG0 | UCRXIFG0);
	// disable RX interrupt (will be enable later)
	UCB0IE &= ~UCRXIE0;
	// enable TX interrupt (will be disabled later)
	UCB0IE |= UCTXIE0;

	// transmit mode and send start condition
	UCB0CTLW0 |= UCTR | UCTXSTT;
}


/* --- interrupts --- */

#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void)
{
	if (UCB0IFG & UCRXIFG0)
	{
		buffer_index = UCB0RXBUF;
		if (rx_count)
		{
			*receive_buffer++ = buffer_index;
			rx_count--;
		}

		// send stop condition on penultimate byte
		if (rx_count == 1)
		{
			UCB0CTLW0 |= UCTXSTP;
		}
		// disable interrupt on last byte
		else if (rx_count == 0)
		{
			UCB0IE &= ~UCRXIE0;
		}
	}
	else if (UCB0IFG & UCTXIFG0)
	{
		switch (mode)
		{
			case SEND_DEVICE_REGISTER:
				UCB0TXBUF = device_register;
				if (rx_count)
				{
					mode = READ_START;
				}
				else
				{
					mode = TRANSMIT;
				}

				break;

			// switch over to read mode
			case READ_START:
				// enable RX interrupt
				UCB0IE |= UCRXIE0;
				// disable TX interrupt
				UCB0IE &= ~UCTXIE0;
				// switch to receiver mode
				UCB0CTLW0 &= ~UCTR;

				mode = READ;

				// send repeated start
				UCB0CTLW0 |= UCTXSTT;
				if (rx_count == 1)
				{
					// send a stop since this is the penultimate byte
					while((UCB0CTLW0 & UCTXSTT));
					
					// send stop condition
					UCB0CTLW0 |= UCTXSTP;
				}

				break;

			case TRANSMIT:
				if (tx_count)
				{
					UCB0TXBUF = transmit_buffer[buffer_index++];
					tx_count--;
				}
				else
				{
					// send stop condition and disable TX interrupt
					UCB0CTLW0 |= UCTXSTP;
					UCB0IE &= ~UCTXIE0;
				}

				break;
		}
	}
}
