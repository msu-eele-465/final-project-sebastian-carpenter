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

// turn the speaker interrupt off or on
// should only be on in playing mode
void clear_speaker_interrupt(void);
void set_speaker_interrupt(void);

// turn the audio display interrupt off or on
//  this is the audio levels displayed on the LED bar
// should not be on while playing or recording
void clear_audio_display_interrupt(void);
void set_audio_display_interrupt(void);

#endif
