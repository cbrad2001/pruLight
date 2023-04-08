/*  buzzer.h
*   
*   Module that defines a background thread to listen for commands regarding when to play a buzzer.
*/

#ifndef _BUZZER_H_
#define _BUZZER_H_

#include <stdint.h>
#include <stdbool.h>

#define C_NOTE_PERIOD 3822256
#define C_NOTE_DUTY_CYCLE 1911128

typedef struct Note {
    bool isNote;    // If it is not a note, then this "plays" nothing
    uint32_t period;
    uint32_t dutyCycle;
    uint32_t durationInMs;
} Note;

void Buzzer_startListening(void);
void Buzzer_stopListening(void);
void Buzzer_addToQueue(Note toPlay);

#endif