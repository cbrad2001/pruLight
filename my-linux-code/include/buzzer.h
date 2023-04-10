/*  buzzer.h
*   Module that defines a background thread to listen for commands regarding when to play a buzzer.
*/
#ifndef _BUZZER_H_
#define _BUZZER_H_

#include <stdint.h>
#include <stdbool.h>

#define C_NOTE_PERIOD 3822256
#define C_NOTE_DUTY_CYCLE 1911128
#define F_NOTE_PERIOD 5727048
#define F_NOTE_DUTY_CYCLE 2863524

typedef struct Note {
    bool isNote;    // If it is not a note, then this "plays" nothing
    uint32_t period;
    uint32_t dutyCycle;
    uint32_t durationInMs;
} Note;

// Starts and stops a background thread that listens to what notes to play to the buzzer.
void Buzzer_startListening(void);
void Buzzer_stopListening(void);

// Adds the passed in list of notes to a buffer for the buzzer to play.
void Buzzer_addToQueue(Note *toPlay, int numToPlay);

#endif