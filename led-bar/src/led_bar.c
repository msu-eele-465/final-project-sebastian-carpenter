#include <msp430fr2355.h>
#include <stdint.h>

void init_led_bar(void){
    // setup P2.6 - P2.7 as outputs
    P2SEL0 &= ~(BIT6 | BIT7);
    P2SEL1 &= ~(BIT6 | BIT7);
    P2OUT &= ~(BIT6 | BIT7);
    P2DIR |= BIT6 | BIT7;

    // setup P3.0 - P3.7 as outputs
    P3SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P3SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P3OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P3DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
}

void update_led_bar(uint16_t pattern){
    // update first two bits of LED bar
    P2OUT &= !(BIT6 | BIT7);
    P2OUT |= (pattern & 0x03) << 6;

    // updated last eight bits of LED bar
    P3OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P3OUT |= (pattern & 0x03FC) >> 2;
}
