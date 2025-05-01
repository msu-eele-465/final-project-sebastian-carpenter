#include <msp430fr2355.h>
#include <stdint.h>

#include "intrinsics.h"
#include "../src/accelerometer.h"

int main(void) {

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // hearbeat LED
    P2SEL0 &= !BIT0;
    P2SEL1 &= !BIT0;
    P2OUT &= ~BIT0;
    P2DIR |= BIT0;

    init_accelerometer_start();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

    init_accelerometer_end();

    __enable_interrupt();

    accelerometer_send_init();

    uint16_t delay;
    while(1)
    {
        // transmit register address with write message
        // put into Tx mode
        UCB0CTLW0 |= UCTR;
        // only send register address
        UCB0TBCNT = 1;
        // generate START condition
        accelerometer_index = 0;
        UCB0CTLW0 |= UCTXSTT;

        // wait for STOP
        while((UCB0IFG & UCSTPIFG) == 0);

        // clear STOP flag
        UCB0IFG &= ~UCSTPIFG;

        // receive data from Rx
        // put into Rx mode
        UCB0CTLW0 &= ~UCTR;
        // receive current time
        UCB0TBCNT = sizeof accelerometer_data - 1;
        // generate START condition
        accelerometer_index = 0;
        UCB0CTLW0 |= UCTXSTT;

        // wait for STOP
        while((UCB0IFG & UCSTPIFG) == 0);

        // clear STOP flag
        UCB0IFG &= ~UCSTPIFG;

        // delay (~1ms)
        for(delay = 0; delay < 1000; delay++);
    }
}

// runs when Tx buffer is ready for data
// or when Rx buffer has received data
// triggered during initialization of accelerometer or during password input
#pragma vector=EUSCI_B0_VECTOR
__interrupt void accelerometer_transmission(void)
{
    // determine interrupt type
    switch (UCB0IV)
    {
        // RXIFG0
        case 0x16:
            // retrieve accelerometer and gyroscope data, byte by byte
            // offset 6 and 7 is temeprature data, so store it but write over it later
            if (accelerometer_index == 6 || accelerometer_index == 7)
            {
                accelerometer_data[accelerometer_index] = UCB0RXBUF;
            }
            else
            {
                accelerometer_data[accelerometer_index++] = UCB0RXBUF;
            }

            break;

        // TXIFG0
        case 0x18:
            // turn off sleep mode
            UCB0TXBUF = ACCELEROMETER_INIT_PACKET[accelerometer_index++];

            break;
    }
}
