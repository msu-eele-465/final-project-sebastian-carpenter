#include <msp430.h>
#include <stdint.h>

#include "./i2c.h"

// it is kind of ridiculous to have so many explicity volatile variables
//	but this is to ensure that i2c works exactly as it is written.
// during testing it was found that rx_count and mode needed the volatile
//	qualifier. The others are untested.

static volatile uint8_t master;

static volatile uint8_t transmit_buffer[16];
static volatile uint8_t *receive_buffer;

static volatile uint8_t device_register;
static volatile uint8_t register_size;

static volatile uint8_t buffer_index;
static volatile uint8_t tx_count;
static volatile uint8_t rx_count;

enum I2C_MODES{
	SEND_DEVICE_REGISTER, TRANSMIT, READ_START, READ,
	SLAVE_RECEIVE_START, SLAVE
};

static volatile enum I2C_MODES mode;


void init_i2c_pins(void)
{
    P1SEL0 |= BIT2 | BIT3;
    P1SEL1 &= ~(BIT2 | BIT3);
}


/* --- master --- */


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

	master = 1;
}

void i2c_write(uint8_t device_address, uint8_t register_address, uint8_t *buffer, uint8_t count)
{
	mode = SEND_DEVICE_REGISTER;

	device_register = register_address;
	buffer_index = 0;
	// this is 255 so that it rolls over once incremented
	//	otherwise the first byte will be wrong and too many
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


/* --- slave --- */


void configure_i2c_slave(uint8_t slave_address, uint8_t *buffer, uint8_t size_of_registers)
{
	// turn on the software reset
    UCB0CTLW0 |= UCSWRST;
    // i2c slave mode
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;
    // set 7-bit address and enable it
	UCB0I2COA0 = (0x7F & slave_address) | UCOAEN;
    // turn off the software reset
    UCB0CTLW0 &= ~UCSWRST;

	// enable RX, TX, and stop interrupt
	UCB0IE |= UCRXIE0 | UCTXIE0 | UCSTPIE;

	master = 0;
	receive_buffer = buffer;
	register_size = size_of_registers;
	mode = SLAVE_RECEIVE_START;
}


/* --- interrupts --- */


#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void)
{
	static volatile uint8_t received_value;

	if(UCB0IFG & UCSTPIFG)
	{
		UCB0IFG &= ~UCSTPIFG;
	}
	else if (UCB0IFG & UCRXIFG0)
	{
		received_value = UCB0RXBUF;

		if (!master)
		{
			switch (mode)
			{
				case SLAVE_RECEIVE_START:
					device_register = received_value;
					buffer_index = register_size * device_register;
					rx_count = register_size;
					tx_count = register_size;
					mode = SLAVE;
					break;
				
				case SLAVE:
					receive_buffer[buffer_index++] = received_value;
					// all bytes received
					if (!(--rx_count))
					{
						mode = SLAVE_RECEIVE_START;
					}
					break;
			}
		}
		else
		{
			if (rx_count)
			{
				*receive_buffer++ = received_value;
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
	}
	else if (UCB0IFG & UCTXIFG0)
	{
		if (!master)
		{
			UCB0TXBUF = receive_buffer[buffer_index++];
			if(!(--tx_count))
			{
				mode = SLAVE_RECEIVE_START;
			}
		}
		else
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
}
