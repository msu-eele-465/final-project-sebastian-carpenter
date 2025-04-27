#ifndef TIMER_INTERRUPTS_H
#define TIMER_INTERRUPTS_H

// initialize Timer B0 at 1MHz with interrupts
// 	CCR0 -> for speaker 1 and the mic
// 	CCR1 -> for speaker 2
void init_timer_interrupts(void);

// turn the mic interrupt off or on
// should only be on in recording mode
void clear_mic_interrupt(void);
void set_mic_interrupt(void);

// turn the speaker interrupts off or on
// should only be on in playing mode
void clear_speaker_interrupts(void);
void set_speaker_interrupts(void);

#endif
